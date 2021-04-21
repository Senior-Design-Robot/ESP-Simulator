#include "kinematics.h"
#include "piwifi.h"

#include <QDebug>


// Private Members
//-----------------------------------------------------------------

bool WPacketBuffer::isWifiHeader()
{
    if( packetByte(0) != 0xFF ) return false;
    if( packetByte(1) != 0xFF ) return false;
    if( packetByte(2) != 0xFD ) return false;
    if( packetByte(3) == 0xFD ) return false; // byte stuffing, not header
    return true;
}

uint32_t WPacketBuffer::read32Val( int start )
{
    uint32_t value = packetByte(start);
    value <<= 8;
    value |= packetByte(start + 1);
    value <<= 8;
    value |= packetByte(start + 2);
    value <<= 8;
    value |= packetByte(start + 3);
    return value;
}

float WPacketBuffer::readPtX( int idx )
{
    double value = read32Val(idx);
    return (float)((value * ((ARM_REACH * 2) / UINT32_MAX)) - ARM_REACH);
}

float WPacketBuffer::readPtY( int idx )
{
    double value = read32Val(idx);
    return (float)(value * (ARM_REACH / UINT32_MAX));
}


// Public Members
//-----------------------------------------------------------------

void WPacketBuffer::reset()
{
    insIndex = 0;
    packetStart = 0;
}

uint8_t WPacketBuffer::packetByte( int idx )
{
    idx = (packetStart + idx) % BUF_LEN;
    return *((uint8_t *)(buf + idx));
}

PathElement WPacketBuffer::packetPoint( int idx )
{
    PathElementType type = static_cast<PathElementType>(packetByte(idx));

    // center x = 0, y = 0 on arm base for kinematics. Left of arm is -x
    float x = readPtX(idx + WPOINT_X_OFFSET);
    float y = readPtY(idx + WPOINT_Y_OFFSET);

    return PathElement(type, x, y);
}

int WPacketBuffer::available()
{
    if( packetStart <= insIndex )
    {
        // simple sequence
        return insIndex - packetStart;
    }
    else
    {
        // loop around
        return packetStart + (BUF_LEN - insIndex);
    }
}

WPacketType WPacketBuffer::seekPacket()
{
    while( available() >= 4 )
    {
        if( isWifiHeader() ) break;
        packetStart = (packetStart + 1) % BUF_LEN;
    }

    int avail = available();
    if( avail < 5 )
    {
        if( !avail )
        {
            insIndex = 0;
            packetStart = 0;
        }
        return WPKT_NULL;
    }
    else
    {
        //Serial.printf("Packet avail: %d\n", avail);

        WPacketType type = (WPacketType)packetByte(4);
        if( type == WPKT_SETTING )
        {
            return (avail >= WPKT_SETTING_LEN) ? WPKT_SETTING : WPKT_NULL;
        }
        if( type == WPKT_POINTS )
        {
            if( avail < WPKT_POINTS_LEN ) return WPKT_NULL;
            int nPts = packetByte(5);
            int totLen = WPKT_POINTS_LEN + nPts * WPOINT_LEN;

            return (avail >= totLen ) ? WPKT_POINTS : WPKT_NULL;
        }
    }

    return WPKT_NULL;
}

WPacketType WPacketBuffer::acceptData( uint8_t *data, int toRead )
{
    int availSpace = BUF_LEN - insIndex;
    int remainRead, totalRead = 0;

    if( toRead > availSpace )
    {
        remainRead = toRead - availSpace;
        toRead = availSpace;
    }
    else
    {
        remainRead = 0;
    }

    memcpy(buf + insIndex, data, toRead);
    totalRead += toRead;

    if( remainRead > 0 )
    {
        // only read up to start of data
        toRead = (remainRead > packetStart) ? packetStart : remainRead;
        memcpy(buf, data + totalRead, toRead);
        insIndex = toRead;
        totalRead += toRead;
    }
    else
    {
        insIndex += toRead;
    }

    qDebug() << "Received " << totalRead << " bytes\n";

    return seekPacket();
}

void WPacketBuffer::munch( int length )
{
    packetStart += length;
    packetStart %= BUF_LEN;
    if( packetStart > insIndex ) packetStart = insIndex;
}


// Receive Wrapper
//-----------------------------------------------------------------

void ReceiveWrapper::handle_incoming_data()
{
    QByteArray data = socket->readAll();
    WPacketType result = buffer->acceptData((uint8_t *)data.data(), data.size());

    if( result == WPKT_POINTS )
    {
        int nPts = buffer->packetByte(WFIELD_N_PTS);
        qDebug() << "Received " << nPts << " points\n";

        for( int i = 0; i < nPts; i++ )
        {
            // process a point/command
            PathElement nextPoint = buffer->packetPoint(WFIELD_POINTS + (WPOINT_LEN * i));

            //Serial.printf("Rcv Point: type=%d: x=%f, y=%f\n", nextPoint.type, nextPoint.x, nextPoint.y);

            emit point_received(nextPoint);
        }

        buffer->munch(WPKT_POINTS_LEN + (WPOINT_LEN * nPts));
    }
    else if( result == WPKT_SETTING )
    {
        int settingID = buffer->packetByte(WFIELD_SETTING_ID);
        int settingValue = buffer->packetByte(WFIELD_SETTING_VAL);

        emit setting_received(settingID, settingValue);

        buffer->munch(WPKT_SETTING_LEN);
    }
}

void ReceiveWrapper::handle_socket_disconn()
{
    socket->deleteLater();
    this->deleteLater();
}
