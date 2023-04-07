#include "joystickdirectinput.h"
#include <stdio.h>

#define SAFE_RELEASE(p)     { if(p) { (p)->Release(); (p) = NULL; } }

Joystick::Joystick(unsigned int id)
{
    this->id = id;
    device_counter = 0;

    di = NULL;
    joystick = NULL;
}

Joystick::~Joystick()
{
    close();
}

HRESULT
Joystick::deviceName(char* name)
{
    HRESULT hr;
    DIDEVICEINSTANCE device;

    ZeroMemory(&device, sizeof(device));
    device.dwSize = sizeof(device);

    if (!di || !joystick) {
        return E_INVALIDARG;
    }

    if (FAILED(hr = joystick->GetDeviceInfo(&device))) {
        return hr;
    }

    strncpy(name, (char*)device.tszProductName, MAX_PATH);

    return hr;
}

HRESULT
Joystick::open()
{
    HRESULT hr;

    // Create a DirectInput device
    if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL),
                                       DIRECTINPUT_VERSION,
                                       IID_IDirectInput8,
                                       (VOID**)&di, NULL))) {
        return hr;
    }

    // Look for the first simple joystick we can find.
    if (FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL, ::enumCallback,
                                    (LPVOID)this, DIEDFL_ATTACHEDONLY))) {
        return hr;

    }

    // Make sure we got a joystick
    if (joystick == NULL) {
        return E_FAIL;
    }

    // Set the data format to "simple joystick" - a predefined data format
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if (FAILED(hr = joystick->SetDataFormat(&c_dfDIJoystick2))) {
        return hr;
    }

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if (FAILED(hr = joystick->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND))) {
        return hr;
    }

    return S_OK;
}

HRESULT
Joystick::close()
{
    if (joystick) {
        joystick->Unacquire();
    }

    SAFE_RELEASE(joystick);
    SAFE_RELEASE(di);

    return S_OK;
}

HRESULT
Joystick::poll(DIJOYSTATE2 *js)
{
    HRESULT hr;

    if (joystick == NULL) {
        return S_OK;
    }

    // Poll the device to read the current state
    hr = joystick->Poll();
    if (FAILED(hr)) {

        // DirectInput is telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so we
        // don't have any special reset that needs to be done.  We just
        // re-acquire and try again.
        hr = joystick->Acquire();
        while (hr == DIERR_INPUTLOST) {
            hr = joystick->Acquire();
        }

        // If we encounter a fatal error, return failure.
        if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
            return E_FAIL;
        }

        // If another application has control of this device, return success.
        // We'll just have to wait our turn to use the joystick.
        if (hr == DIERR_OTHERAPPHASPRIO) {
            return S_OK;
        }
    }

    // Get the input's device state
    if (FAILED(hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), js))) {
        return hr;
    }

    return S_OK;
}

BOOL CALLBACK
Joystick::enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
    // If this is the requested device ID ...
    if (device_counter == this->id) {

        // Obtain an interface to the enumerated joystick.  Stop the enumeration
        // if the requested device was created successfully.
        if (SUCCEEDED(di->CreateDevice(instance->guidInstance, &joystick, NULL))) {
            return DIENUM_STOP;
        }
    }

    // Otherwise, increment the device counter and continue with
    // the device enumeration.
    device_counter++;

    return DIENUM_CONTINUE;
}

BOOL CALLBACK
enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
    if (context != NULL) {
        return ((Joystick *)context)->enumCallback(instance, context);
    } else {
        return DIENUM_STOP;
    }
}

unsigned int
Joystick::deviceCount()
{
    unsigned int counter = 0;
    LPDIRECTINPUT8 di = NULL;
    HRESULT hr;

    if (SUCCEEDED(hr = DirectInput8Create(GetModuleHandle(NULL),
                                          DIRECTINPUT_VERSION,
                                          IID_IDirectInput8,
                                          (VOID**)&di, NULL))) {
        di->EnumDevices(DI8DEVCLASS_GAMECTRL, ::countCallback,
                        &counter, DIEDFL_ATTACHEDONLY);
    }

    return counter;
}

BOOL CALLBACK
countCallback(const DIDEVICEINSTANCE* instance, VOID* counter)
{
    if (counter != NULL) {
        unsigned int *tmpCounter = (unsigned int *)counter;
        (*tmpCounter)++;
        counter = tmpCounter;
    }

    return DIENUM_CONTINUE;
}
