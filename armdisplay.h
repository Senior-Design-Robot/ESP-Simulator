#pragma once

#include <vector>
#include <QWidget>
#include "kinematics.h"

class ArmDisplay : public QWidget
{
    Q_OBJECT
private:
    struct arm_angles angles;
    struct arm_angles angles2;

    static constexpr float REAL_WIDTH = LEN_A + LEN_B;

public:
    std::vector<QPointF> pastPoints;
    std::vector<QPointF> pastPoints2;
    bool penDown;
    bool penDown2;

    explicit ArmDisplay(QWidget *parent = nullptr);

    void setArmPosition( struct arm_angles ang, float tipX, float tipY );
    void setArm2Position( struct arm_angles ang, float tipX, float tipY );
    void reset();

protected:
    void paintEvent( QPaintEvent * );

};
