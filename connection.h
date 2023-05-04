#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QDataStream>
#include <QNetworkInterface>
#include <QHostInfo>

class Connection : public QObject
{
    Q_OBJECT
private:
    QUdpSocket *socket;

    QHostAddress hostAdress;
    int hostPort;

    QHostAddress receivingAdress;
    int receivingPort;

    QByteArray frame;
    int bytesSentInLastFrame;
    const char DEFAULT_VALUE = 0;


public:
    explicit Connection(QObject *parent = nullptr);
    ~Connection();

    long long int framesSent;
    long long int framesReceived;

    float azimuth;
    double latitude;
    double longitude;

    QString currentIP;

public slots:
    bool createFrame(const QByteArray &frameDrive, const QByteArray &frameArm);
    bool sendFrame();
    void setHostAddress(const QString &address);
    void setHostPort(quint16 port);
    void processReceivedDatagram();
    QString localIpAddress();
    QString byteArrayToBinaryString(const QByteArray& byteArray);
    float bytesToFloat(uchar b0, uchar b1, uchar b2, uchar b3);

signals:
    void frameCreated();
    void frameSent();
    void frameReceived();
    void hostAddressChanged(const QString &address);
    void hostPortChanged(quint16 port);
    void frameFailedToBeSent(int value);
    void currentIpChanged(const QString &value);
};

#endif // CONNECTION_H
