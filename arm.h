#ifndef ARM_H
#define ARM_H

#include <QObject>
#include <QIODevice>
#include <QDebug>


class Arm : public QObject
{
    Q_OBJECT
public:

    explicit Arm(QObject *parent = 0);

    QByteArray frame;

    enum ButtonFunction
    {
        none,
        first,
        second,
        third,
        jaws,
        all,
        inverseKinematics
    };

    int containerX;
    int containerY;
    int containerZ;
    int containerPower;
    ButtonFunction buttonFunction;
    int buttonPressed;

    bool calculateSegmentsSpeeds(const int x, const int y, const int z, const int power, ButtonFunction buttonFunction, int buttonPressed);

private:

    int16_t motorBase;
    int16_t motorSegment2Middle;
    int16_t motorSegment1Bottom;
    int16_t motorJawsPosition;
    int16_t motorJawsRotation;
    int16_t motorJawsClench;

    double joyX = 0.00;
    double joyY = 0.00;
    double joyZ = 0.00;

    int powerOnMotors = 0;

    double deadzone = 0.25;

    int motorX = 0;
    int motorY = 0;
    int motorZ = 0;

public slots:

signals:
    void controlVirtualSliders();
    void controlPhysicalJoystick();
};
#endif // ARM_H
