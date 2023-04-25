#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QUdpSocket>
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
    void setHostAddress(const QString &address);
    void setHostPort(quint16 port);

signals:
    void frameCreated();
    void frameSent();
    void hostAddressChanged(const QString &address);
    void hostPortChanged(quint16 port);
};

#endif // CONNECTION_H
