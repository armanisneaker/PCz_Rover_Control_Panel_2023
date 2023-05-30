#ifndef ARM_H
#define ARM_H

#include <QObject>
#include <QIODevice>

class Arm : public QObject
{
    Q_OBJECT
public:
    explicit Arm(QObject *parent = nullptr);

    QByteArray frame;
    QByteArray framePoker;

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
    void calculatePokerValues();
    int containerX;
    int containerY;
    int containerZ;
    int containerPower;
    ButtonFunction activeButtonFunction;
    int buttonPressed;
    static constexpr int32_t joystickCenter = 32767;

    uint16_t pokerAngle;
    uint16_t pokerPoke;

private:


    int16_t motorBase;
    int16_t motorSegment2Middle;
    int16_t motorSegment1Bottom;
    int16_t motorJawsPosition;
    int16_t motorJawsRotation;
    int16_t motorJawsClench;

    int16_t prevMotorBase;
    int16_t prevMotorSegment1Bottom;
    int16_t prevMotorSegment2Middle;
    int16_t prevMotorJawsClench;
    int16_t prevMotorJawsRotation;
    int16_t prevMotorJawsPosition;

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
    void motorBaseChanged(int value);
    void motorFirstChanged(int value);
    void motorSecondChanged(int value);
    void motorThirdChanged(int value);
    void motorJawsChanged(int value);
    void motorJawsClenchChanged(int value);
};

#endif // ARM_H
