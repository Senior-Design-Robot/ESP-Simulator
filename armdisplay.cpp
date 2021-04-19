#include "armdisplay.h"
#include <cmath>
#include <QPainter>

ArmDisplay::ArmDisplay(QWidget *parent) : QWidget(parent)
{
    angles.shoulder = M_PI_2;
    angles.elbow = -M_PI;

    angles2.shoulder = M_PI_2;
    angles2.elbow = -M_PI;

    penDown = false;
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
    if( penDown2 ) pastPoints2.push_back(QPointF(tipX, tipY));

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

    QPointF center = canvas.center();
    center.setX(center.x() + 225);

    QPointF center2 = canvas.center();
    center2.setX((center2.x() - 225));

    float scale = (float)(qMin(canvas.width(), canvas.height()) / (2 * (REAL_WIDTH)));
    float asize = LEN_A * scale;
    float bsize = LEN_B * scale;

    for( QPointF &pt : pastPoints )
    {
        QPointF adj = (pt * scale) + center;
        painter.setPen(QPen(QColor(0,0,0), 2));
        painter.drawPoint(adj);
    }

    float ax = center.x() + asize * cosf(angles.shoulder);
    float ay = center.y() - asize * sinf(angles.shoulder);
    QPointF a_tip(ax, ay);

    float bx = ax + bsize * cosf(angles.shoulder + angles.elbow);
    float by = ay - bsize * sinf(angles.shoulder + angles.elbow);
    QPointF b_tip(bx, by);

    painter.setPen(QColor(255, 0, 0));
    painter.drawLine(center, a_tip);

    painter.setPen(QColor(0, 0, 255));
    painter.drawLine(a_tip, b_tip);

    //arm 2
    float ax2 = center2.x() + asize + cosf(angles2.shoulder) - 147;
    float ay2 = center2.y() - asize * sinf(angles2.shoulder);
    QPointF a_tip2(ax2, ay2);

    float bx2 = ax2 + bsize * cosf(angles2.shoulder + angles2.elbow);
    float by2 = ay2 - bsize * sinf(angles2.shoulder + angles2.elbow);
    QPointF b_tip2(bx2, by2);

    painter.setPen(QColor(255, 255, 0));
    painter.drawLine(center2, a_tip2);

    painter.setPen(QColor(0, 255, 255));
    painter.drawLine(a_tip2, b_tip2);


}
