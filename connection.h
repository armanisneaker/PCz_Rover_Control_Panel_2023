#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QUdpSocket>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

class Connection : public QObject
{
    Q_OBJECT
private:
    QUdpSocket *socket;
    QHostAddress hostAdress;
    int hostPort;

    QByteArray frame;
    int bytesSentInLastFrame;

public:
    explicit Connection(QObject *parent = nullptr);
    ~Connection();

    long long int framesSent;

public slots:
    bool createFrame(const QByteArray &frameDrive, const QByteArray &frameArm);
    bool sendFrame();

signals:
    void frameCreated();
    void frameSent();
};

#endif // CONNECTION_H
