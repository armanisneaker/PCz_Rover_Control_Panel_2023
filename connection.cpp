#include "connection.h"

Connection::Connection(QObject *parent)
    : QObject{parent}
{
    hostAdress = QHostAddress("192.168.1.10");
    hostPort = 5150;
    frame.resize(320);
    framesSent = 0;

    socket = new QUdpSocket(this);
    socket->bind(hostAdress,hostPort);

    connect(this, &Connection::frameCreated, this, &Connection::sendFrame);
}

Connection::~Connection()
{

}

Q_INVOKABLE bool Connection::createFrame(const QByteArray& frameDrive, const QByteArray& frameArm)
{
    // Check that the frame has enough space to accommodate the drive and arm frames
    if (frame.size() < 67 + frameArm.size())
    {
        qWarning() << "Frame size is too small for the drive and arm frames.";
        return false;
    }

    // Copy the frameDrive data to the appropriate location in the frame
    memcpy(frame.data() + 4, frameDrive.constData(), frameDrive.size());

    // Copy the frameArm data to the appropriate location in the frame
    memcpy(frame.data() + 67, frameArm.constData(), frameArm.size());

    emit frameCreated();
    return true;
}

bool Connection::sendFrame()
{
    bytesSentInLastFrame = socket->writeDatagram(frame,hostAdress,hostPort);

    if(bytesSentInLastFrame == 320)
    {
        qDebug() << "Frame successfully sent to"<< hostAdress << hostPort << ".";
        framesSent++;
        emit frameSent();
        return true;
    }
    else
    {
        qDebug() << "Frame not sent. It's "<<bytesSentInLastFrame<<" bytes long.";
        return false;
    }
}
