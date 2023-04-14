// DirectInputJoystick.h

#ifndef DIRECTINPUTJOYSTICK_H
#define DIRECTINPUTJOYSTICK_H

#include <QObject>
#include <QTimer>
#include <dinput.h>

class DirectInputJoystick : public QObject
{
    Q_OBJECT

public:
    explicit DirectInputJoystick(QObject *parent = nullptr);
    ~DirectInputJoystick();

    bool initialize();
    void update();
    int connectedJoystickCount() const;

signals:
    void joystick1AxisXChanged(int value);
    void joystick1AxisYChanged(int value);
    void joystick1AxisZChanged(int value);
    void joystick1SliderChanged(int value);
    void joystick2AxisXChanged(int value);
    void joystick2AxisYChanged(int value);
    void joystick2AxisZChanged(int value);
    void joystick2SliderChanged(int value);
    void joystickButtonStateChanged(int joystick, int button, bool pressed);
    void connectedJoystickCountChanged(int count);


private:
    static BOOL CALLBACK enumDevicesCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
    HRESULT createDevice(IDirectInput8 *pDI, const DIDEVICEINSTANCE *pdidInstance);
    void pollDevices();
    void processDeviceInput(int deviceIndex, DIJOYSTATE &currentState);

    IDirectInput8 *m_directInput;
    IDirectInputDevice8 *m_joysticks[2];
    QTimer *m_updateTimer;
    int m_previousJoystickCount;
    QTimer *m_joystickCountTimer;
    DIJOYSTATE m_previousDeviceState[2];


private slots:
    void checkJoystickCount();
};

#endif // DIRECTINPUTJOYSTICK_H
