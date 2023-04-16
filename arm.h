#ifndef ARM_H
#define ARM_H

#include <QObject>
#include <QIODevice>
#include <QDebug>

class Arm : public QObject
{
    Q_OBJECT
public:
    explicit Arm(QObject *parent = nullptr);

    QByteArray frame;

    enum ButtonFunction
    {
        None = 0,
        First = 1,
        Second = 2,
        Third = 3,
        Jaws = 4,
        All = 5,
        InverseKinematics = 6
    };

    struct ButtonFunctionKey
    {
        ButtonFunction function;
        bool isActive;

        ButtonFunctionKey() : function(Arm::None), isActive(false) {}
    };

    ButtonFunctionKey buttonFunctionKeys[6];
    bool calculateSegmentsSpeeds(int x, int y, int z, int power, ButtonFunction buttonFunction, int buttonPressed);
    int containerX;
    int containerY;
    int containerZ;
    int containerPower;
    ButtonFunction activeButtonFunction;
    int buttonPressed;

private:


    int16_t motorBase;
    int16_t motorSegment2Middle;
    int16_t motorSegment1Bottom;
    int16_t motorJawsPosition;
    int16_t motorJawsRotation;
    int16_t motorJawsClench;

    double joyX;
    double joyY;
    double joyZ;

    int powerOnMotors;

    double deadzone;

    int motorX;
    int motorY;
    int motorZ;

public slots:
    void processButtonPressed(int buttonPressedNow);
    void onButtonFunctionIndexChanged(int index, int newIndex);

signals:
    void controlVirtualSliders();
    void controlPhysicalJoystick1();
    void controlPhysicalJoystick2();
    void buttonFunctionChanged(int index);
};

#endif // ARM_H
