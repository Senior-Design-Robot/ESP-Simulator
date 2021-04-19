#pragma once

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

#include <stdint.h>
#include "pathiterator.h"

#define WPKT_HEAD_LEN 5
#define WPKT_SETTING_LEN (WPKT_HEAD_LEN + 2)
#define WPKT_POINTS_LEN (WPKT_HEAD_LEN + 1)

#define WFIELD_PKT_TYPE 4
#define WFIELD_SETTING_ID 5
#define WFIELD_SETTING_VAL 6

#define WFIELD_N_PTS 5
#define WFIELD_POINTS 6

#define WPOINT_LEN 9
#define WPOINT_X_OFFSET 1
#define WPOINT_Y_OFFSET 5

enum WPacketType : char
{
    WPKT_NULL = 0,
    WPKT_SETTING = 1,
    WPKT_POINTS = 2
};


class WPacketBuffer
{
private:
    static const int BUF_LEN = 512;

    char buf[BUF_LEN];
    int insIndex = 0;
    int packetStart = 0;

    bool isWifiHeader();
    uint32_t read32Val( int start );
    float readPtX( int idx );
    float readPtY( int idx );

public:
    /** Flush any data in the buffer to handle a new connection */
    void reset();

    /** Get the byte value at the give offset in the current packet */
    uint8_t packetByte( int idx );

    /** Get the point struct starting at the given offset */
    PathElement packetPoint( int idx );

    /** Return the number of bytes waiting in the buffer */
    int available();

    /** Advance the ptr to the first valid packet & return the type */
    WPacketType seekPacket();

    /** Recieve waiting data from a connection */
    WPacketType acceptData( uint8_t *data, int len );

    /** Jump the packet ptr forward the given length */
    void munch( int length );
};


class ReceiveWrapper : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *socket;
    WPacketBuffer *buffer;

private slots:
    void handle_incoming_data();
    void handle_socket_disconn();

public:
    ReceiveWrapper( QTcpSocket *sock, WPacketBuffer *buf, QObject *parent ) : QObject(parent), socket(sock), buffer(buf) {}

signals:
    void point_received( PathElement elem );
};


class TransmitWrapper : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *socket;
    QByteArray data;
    int send_length = 0;
    int bytes_sent = 0;

    void on_connected() { socket->write(data.data(), data.size()); }
    void on_error()
    {
        socket->close();
        this->deleteLater();
    }

    void on_write( int n_bytes )
    {
        bytes_sent += n_bytes;
        if( bytes_sent >= send_length )
        {
            socket->close();
            this->deleteLater();
        }
    }

public:
    TransmitWrapper( QObject *parent ) : QObject(parent)
    {
        socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::connected, this, &TransmitWrapper::on_connected);
        connect(socket, &QTcpSocket::errorOccurred, this, &TransmitWrapper::on_error);
        connect(socket, &QTcpSocket::bytesWritten, this, &TransmitWrapper::on_write);
    }

    void start_transmit( QHostAddress addr, QByteArray bytes )
    {
        data = bytes;
        send_length = bytes.size();
        socket->connectToHost(addr, 1896);
    }
};

