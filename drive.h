#ifndef DRIVE_H
#define DRIVE_H

#include <QObject>
#include <QIODevice>
#include <QDebug>

class Drive : public QObject
{
    Q_OBJECT
public:

    explicit Drive(QObject *parent = 0);

    QByteArray frame;

    int16_t getLeftSpeed() const;
    int16_t getRightSpeed() const;

    int containerX;
    int containerY;
    int containerPower;

private:
    const int MAX_SPEED = 3200;
    const int MAX_JOYSTICK_VALUE = 32767;
    const int DEADZONE = 2000;
    const double WHEELBASE = 2;
    double normX;
    double normY;
    double v;
    double w;
    int16_t leftSpeed;
    int16_t rightSpeed;
    int adjustedX;
    int adjustedY;

public slots:
    bool calculateWheelsSpeeds(const int x, const int y, const int power);
signals:
    void controlVirtualJoystick();
    void controlPhysicalJoystick1();
    void controlPhysicalJoystick2();

};

#endif // DRIVE_H
