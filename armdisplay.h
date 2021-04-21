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

    static constexpr float REAL_WIDTH = (2 * ARM_REACH) + abs(ARM1_X) + abs(ARM2_X);
    static constexpr float REAL_HEIGHT = ARM_REACH;

public:
    std::vector<QPointF> pastPoints;
    std::vector<QPointF> pastPoints2;
    bool penDown;
    bool penDown2;
    bool singleArmMode = false;

    explicit ArmDisplay(QWidget *parent = nullptr);

    void setArmPosition( struct arm_angles ang, float tipX, float tipY );
    void setArm2Position( struct arm_angles ang, float tipX, float tipY );
    void reset();

protected:
    void paintEvent( QPaintEvent * );

};
