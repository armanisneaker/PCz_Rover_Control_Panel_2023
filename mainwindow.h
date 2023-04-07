#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <vector>
#include "connection.h"
#include "drive.h"
#include "joystickwidget.h"
#include "joystickdirectinput.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    Connection* connection;
    Drive* drive;
    JoystickWidget* joystickWidget;
    std::vector<Joystick> joystickPhysical;

    QTimer *connectionTimer;
    QTimer *driveTimer;
    QTimer *uiTimer;
    QTimer* frameStatusTimer;
    QTimer* joystickPhysicalTimer;
    QElapsedTimer* lastFrameSentTime;

    int timeSinceLastFrameSent;

    QMetaObject::Connection connConnectionTimer;
    QMetaObject::Connection connDriveTimer;
    QMetaObject::Connection connJoystickPhysicalTimer;
    QMetaObject::Connection connDriveControlVirtual;
    QMetaObject::Connection connDriveControlPhysical;

    DIJOYSTATE2 joystickState[4];
    unsigned int numJoysticks = 0;
    unsigned int numJoysticksOld = 0;
    int joystickNumber;
    float physicalJoystickMaxValue = 65535;

private slots:
    void updateFrameStatusLabel();
    void on_comboBox_drive_control_currentTextChanged(const QString &arg1);
};
#endif // MAINWINDOW_H
