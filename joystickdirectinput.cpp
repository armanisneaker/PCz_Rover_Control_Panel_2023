// DirectInputJoystick.cpp

#include "joystickdirectinput.h"
#include <QDebug>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

DirectInputJoystick::DirectInputJoystick(QObject *parent)
    : QObject(parent), m_directInput(nullptr), m_updateTimer(new QTimer(this)), m_previousJoystickCount(0), m_joystickCountTimer(new QTimer(this))
{
    for (int i = 0; i < 2; ++i)
        m_joysticks[i] = nullptr;

    if (initialize())
    {
        connect(m_updateTimer, &QTimer::timeout, this, &DirectInputJoystick::update);
        m_updateTimer->start(1000 / 60); // Poll at 60Hz
    }

    connect(m_joystickCountTimer, &QTimer::timeout, this, &DirectInputJoystick::checkJoystickCount);
    m_joystickCountTimer->start(500);  // Check every 1 second
}

DirectInputJoystick::~DirectInputJoystick()
{
    for (int i = 0; i < 2; ++i)
    {
        if (m_joysticks[i])
        {
            m_joysticks[i]->Unacquire();
            m_joysticks[i]->Release();
        }
    }

    if (m_directInput)
        m_directInput->Release();

        delete m_joystickCountTimer;

}

bool DirectInputJoystick::initialize()
{
    releaseJoysticks();

    HRESULT hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID **)&m_directInput, nullptr);
    if (FAILED(hr))
    {
        qWarning() << "Failed to initialize DirectInput.";
        return false;
    }

    hr = m_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
    if (FAILED(hr))
    {
        qWarning() << "Failed to enumerate DirectInput devices.";
        return false;
    }

    return true;
}


void DirectInputJoystick::update()
{
    pollDevices();
}

BOOL CALLBACK DirectInputJoystick::enumDevicesCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
    DirectInputJoystick *pThis = static_cast<DirectInputJoystick *>(pContext);

    if (pThis->createDevice(pThis->m_directInput, pdidInstance) == DI_OK)
        return DIENUM_STOP;

    return DIENUM_CONTINUE;
}

HRESULT DirectInputJoystick::createDevice(IDirectInput8 *pDI, const DIDEVICEINSTANCE *pdidInstance)
{
    HRESULT hr;
    IDirectInputDevice8 *pDevice;

    hr = pDI->CreateDevice(pdidInstance->guidInstance, &pDevice, nullptr);
    if (FAILED(hr))
        return hr;

    hr = pDevice->SetDataFormat(&c_dfDIJoystick);
    if (FAILED(hr))
    {
        pDevice->Release();
        return hr;
    }

    hr = pDevice->SetCooperativeLevel(nullptr, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    if (FAILED(hr))
    {
        pDevice->Release();
        return hr;
    }

    for (int i = 0; i < 2; ++i)
    {
        if (m_joysticks[i] == nullptr)
        {
            m_joysticks[i] = pDevice;
            pDevice->Acquire();
            return DI_OK;
        }
    }

    pDevice->Release();
    return DI_NOEFFECT;
}

// DirectInputJoystick.cpp

void DirectInputJoystick::pollDevices()
{
    for (int i = 0; i < 2; ++i)
    {
        if (m_joysticks[i] == nullptr)
            continue;

        HRESULT hr = m_joysticks[i]->Poll();
        if (FAILED(hr))
            m_joysticks[i]->Acquire();

        DIJOYSTATE currentState;
        hr = m_joysticks[i]->GetDeviceState(sizeof(DIJOYSTATE), &currentState);

        if (SUCCEEDED(hr))
        {
            processDeviceInput(i, currentState);
        }
    }
}

void DirectInputJoystick::processDeviceInput(int deviceIndex, DIJOYSTATE &currentState)
{
    // Process X-axis
    if (currentState.lX != m_previousDeviceState[deviceIndex].lX)
    {
        if (deviceIndex == 0)
        {
            emit joystick1AxisXChanged(currentState.lX);
        }
        else if (deviceIndex == 1)
        {
            emit joystick2AxisXChanged(currentState.lX);
        }
        m_previousDeviceState[deviceIndex].lX = currentState.lX;
    }

    // Process Y-axis
    if (currentState.lY != m_previousDeviceState[deviceIndex].lY)
    {
        if (deviceIndex == 0)
            emit joystick1AxisYChanged(currentState.lY);
        else
            emit joystick2AxisYChanged(currentState.lY);

        m_previousDeviceState[deviceIndex].lY = currentState.lY;
    }

    // Process Z-axis
    if (currentState.lRz != m_previousDeviceState[deviceIndex].lRz)
    {
        if (deviceIndex == 0)
            emit joystick1AxisZChanged(currentState.lRz);
        else
            emit joystick2AxisZChanged(currentState.lRz);

        m_previousDeviceState[deviceIndex].lRz = currentState.lRz;
    }

    // Process Slider
    if (currentState.rglSlider[0] != m_previousDeviceState[deviceIndex].rglSlider[0])
    {
        if (deviceIndex == 0)
            emit joystick1SliderChanged(currentState.rglSlider[0]);
        else
            emit joystick2SliderChanged(currentState.rglSlider[0]);

        m_previousDeviceState[deviceIndex].rglSlider[0] = currentState.rglSlider[0];
    }

    // Process Buttons
        for (int button = 0; button < 32; button++)
        {
            bool currentStatePressed = (currentState.rgbButtons[button] & 0x80) != 0;
            bool previousStatePressed = (m_previousDeviceState[deviceIndex].rgbButtons[button] & 0x80) != 0;

            if (currentStatePressed != previousStatePressed)
            {
                if (deviceIndex == 0)
                    emit joystick1ButtonStateChanged(button, currentStatePressed);
                else
                    emit joystick2ButtonStateChanged(button, currentStatePressed);

                // Emit button released signal
                if ((previousStatePressed && !currentStatePressed) && button >=0 && button <=3)
                {
                    //qDebug() <<button;
                    if (deviceIndex == 0)
                        emit joystick1ButtonReleased(button);
                    else
                        emit joystick2ButtonReleased(button);
                }

                m_previousDeviceState[deviceIndex].rgbButtons[button] = currentState.rgbButtons[button];
            }
        }
}




int DirectInputJoystick::connectedJoystickCount() const
{
    int count = 0;

    for (int i = 0; i < 2; ++i)
    {
        if (m_joysticks[i] != nullptr)
            count++;
    }

    return count;
}

void DirectInputJoystick::checkJoystickCount()
{
    HRESULT hr = m_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumDevicesCallbackUpdate, this, DIEDFL_ATTACHEDONLY);
    if (FAILED(hr))
    {
        qWarning() << "Failed to enumerate DirectInput devices during checkJoystickCount.";
    }

    int currentCount = connectedJoystickCount();

    if (currentCount != m_previousJoystickCount)
    {
        m_previousJoystickCount = currentCount;
        emit connectedJoystickCountChanged(currentCount);
    }
}


void DirectInputJoystick::releaseJoysticks()
{
    for (int i = 0; i < 2; ++i)
    {
        if (m_joysticks[i])
        {
            m_joysticks[i]->Unacquire();
            m_joysticks[i]->Release();
            m_joysticks[i] = nullptr;
        }
    }
}

BOOL CALLBACK DirectInputJoystick::enumDevicesCallbackUpdate(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
    DirectInputJoystick *pThis = static_cast<DirectInputJoystick *>(pContext);
    bool deviceFound = false;

    for (int i = 0; i < 2; ++i)
    {
        if (pThis->m_joysticks[i] != nullptr)
        {
            DIDEVICEINSTANCE instance;
            ZeroMemory(&instance, sizeof(DIDEVICEINSTANCE));
            instance.dwSize = sizeof(DIDEVICEINSTANCE);

            if (pThis->m_joysticks[i]->GetDeviceInfo(&instance) == DI_OK && instance.guidInstance == pdidInstance->guidInstance)
            {
                deviceFound = true;
                break;
            }
        }
    }

    if (!deviceFound)
    {
        pThis->createDevice(pThis->m_directInput, pdidInstance);
    }

    return DIENUM_CONTINUE;
}
