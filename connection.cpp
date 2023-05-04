#include "connection.h"

Connection::Connection(QObject *parent)
    : QObject{parent}
{
    hostAdress = QHostAddress("192.168.1.10");
    hostPort = 5150;
    frame.resize(320);
    frame.fill(DEFAULT_VALUE);
    framesSent = 0;

    socket = new QUdpSocket(this);
    socket->bind(hostAdress,hostPort);

    connect(this, &Connection::frameCreated, this, &Connection::sendFrame);
}

Connection::~Connection()
{

}

bool Connection::createFrame(const QByteArray& frameDrive, const QByteArray& frameArm)
{
    // Check that the frame has enough space to accommodate the drive and arm frames
    if (frame.size() < 67 + frameArm.size())
    {
        qWarning() << "Frame size is too small for the drive and arm frames.";
        return false;
    }

    frame.fill(DEFAULT_VALUE);

    qDebug() << frameDrive + "\n";
    qDebug() << frameArm + "\n";

    QDataStream stream(&frame, QIODevice::WriteOnly);

    // Move the stream position to the start of the frameDrive data location
    stream.device()->seek(4);
    stream.writeRawData(frameDrive.constData(), frameDrive.size());

    // Move the stream position to the start of the frameArm data location
    stream.device()->seek(67);
    stream.writeRawData(frameArm.constData(), frameArm.size());

    emit frameCreated();
    return true;
}


bool Connection::sendFrame()
{
    bytesSentInLastFrame = socket->writeDatagram(frame,hostAdress,hostPort);

    if(bytesSentInLastFrame == 320)
    {
        //qDebug() << "Frame successfully sent to"<< hostAdress << hostPort << ".";
        framesSent++;
        emit frameSent();
        return true;
    }
    else
    {
        emit frameFailedToBeSent(bytesSentInLastFrame);
         qDebug() << frame << "\n" << frame.size();
        return false;
    }

}

void Connection::setHostAddress(const QString &address)
{
    QHostAddress newAddress(address);
    if (newAddress != hostAdress)
    {
        hostAdress = newAddress;
        emit hostAddressChanged(address);
    }
}

void Connection::setHostPort(quint16 port)
{
    if (port != hostPort)
    {
        hostPort = port;
        emit hostPortChanged(port);
    }
}
