#include "armdisplay.h"
#include <cmath>
#include <QPainter>

ArmDisplay::ArmDisplay(QWidget *parent) : QWidget(parent)
{
    angles.shoulder = M_PI_2;
    angles.elbow = -M_PI;

    angles2.shoulder = -M_PI_2;
    angles2.elbow = 2 * M_PI_2;

    penDown = false;
    penDown2 = false;
}

void ArmDisplay::setArmPosition( struct arm_angles ang, float tipX, float tipY )
{
    angles = ang;
    if( penDown ) pastPoints.push_back(QPointF(tipX, -tipY));

    update();
}

void ArmDisplay::setArm2Position(struct arm_angles ang, float tipX, float tipY)
{
    angles2 = ang;
    if( penDown2 ) pastPoints2.push_back(QPointF(tipX, -tipY));

    update();
}

void ArmDisplay::reset()
{
    pastPoints.clear();
    pastPoints2.clear();
    update();
}

void ArmDisplay::paintEvent( QPaintEvent * )
{
    QPainter painter(this);
    QRectF canvas = rect();

    float scale = (float)(qMin(canvas.width() / REAL_WIDTH, canvas.height() / REAL_HEIGHT));
    float asize = LEN_A * scale;
    float bsize = LEN_B * scale;

    QPointF center(canvas.width() / 2, canvas.height());

    QPointF arm1Base = center;
    arm1Base.setX(arm1Base.x() + (scale * ARM1_X));

    QPointF arm2Base = center;
    arm2Base.setX(arm2Base.x() + (scale * ARM2_X));

    painter.setPen(QPen(QColor(0,0,0), 2));
    for( QPointF &pt : pastPoints )
    {
        QPointF adj = (pt * scale) + center;
        painter.drawPoint(adj);
    }

    if( !singleArmMode )
    {
        for( QPointF &pt : pastPoints2 )
        {
            QPointF adj = (pt * scale) + center;
            painter.drawPoint(adj);
        }
    }

    float ax = arm1Base.x() + asize * cosf(angles.shoulder);
    float ay = arm1Base.y() - asize * sinf(angles.shoulder);
    QPointF a_tip(ax, ay);

    float bx = ax + bsize * cosf(angles.shoulder + angles.elbow);
    float by = ay - bsize * sinf(angles.shoulder + angles.elbow);
    QPointF b_tip(bx, by);

    painter.setPen(QPen(QColor(255, 0, 0), 3));
    painter.drawLine(arm1Base, a_tip);

    painter.setPen(QPen(QColor(0, 0, 255), 3));
    painter.drawLine(a_tip, b_tip);

    //arm 2
    if( !singleArmMode )
    {
        float ax2 = arm2Base.x() + asize * cosf(angles2.shoulder + angles2.elbow);
        float ay2 = arm2Base.y() - asize * sinf(angles2.shoulder + angles2.elbow);
        QPointF a_tip2(ax2, ay2);

        float bx2 = ax2 + bsize * cosf(angles2.shoulder);
        float by2 = ay2 - bsize * sinf(angles2.shoulder);
        QPointF b_tip2(bx2, by2);

        painter.setPen(QPen(QColor(255, 128, 0), 3));
        painter.drawLine(arm2Base, a_tip2);

        painter.setPen(QPen(QColor(0, 255, 0), 3));
        painter.drawLine(a_tip2, b_tip2);
    }
}
