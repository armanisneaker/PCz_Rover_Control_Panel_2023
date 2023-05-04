#include "connection.h"

Connection::Connection(QObject *parent)
    : QObject{parent}
{
    hostAdress = QHostAddress("192.168.1.10");
    hostPort = 5150;

    receivingAdress = QHostAddress(localIpAddress());
    receivingPort = 5159;

    frame.resize(320);
    frame.fill(DEFAULT_VALUE);
    framesSent = 0;
    framesReceived = 0;

    azimuth = 0.0;
    longitude = 0.0;
    latitude = 0.0;

    socket = new QUdpSocket(this);
        if (!socket->bind(receivingAdress, receivingPort)) {
            qDebug() << "Failed to bind the socket to" << receivingAdress.toString() << "on port" << receivingPort;
        }

    connect(this, &Connection::frameCreated, this, &Connection::sendFrame);

    connect(socket, &QUdpSocket::readyRead, this, &Connection::processReceivedDatagram);

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

    //qDebug() << frameDrive + "\n";
   // qDebug() << frameArm + "\n";

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
        // qDebug() << frame << "\n" << frame.size();
        return false;
    }

}
float Connection::bytesToFloat(uchar b0, uchar b1, uchar b2, uchar b3)
{
    float output;

    *((uchar*)(&output) + 3) = b0;
    *((uchar*)(&output) + 2) = b1;
    *((uchar*)(&output) + 1) = b2;
    *((uchar*)(&output) + 0) = b3;

    return output;
}

void Connection::processReceivedDatagram()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size());

        if(datagram.size() <= 20)
        {
                // Assuming the float value starts at the first byte (0-th index) of the QByteArray
                int floatIndex = 0;
                // Assuming the first double value starts after the float (4-th index)
                int firstDoubleIndex = floatIndex + sizeof(float);
                // Assuming the second double value starts after the first double (12-th index)
                int secondDoubleIndex = firstDoubleIndex + sizeof(double);

                // Ensure there are enough bytes in the datagram for a float and two doubles
                if (datagram.size() >= secondDoubleIndex + static_cast<int>(sizeof(double)))
                {
                    QDataStream stream(&datagram, QIODevice::ReadOnly);
                    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);

                    // Set the byte order if necessary (e.g., QDataStream::LittleEndian or QDataStream::BigEndian)
                    // stream.setByteOrder(QDataStream::LittleEndian);

                    // Read the float value from the QByteArray
                    float floatValue;
                    stream.device()->seek(floatIndex);
                    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
                    stream >> floatValue;

                    // Read the first double value from the QByteArray
                    double firstDoubleValue;
                    stream.device()->seek(firstDoubleIndex);
                    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
                    stream >> firstDoubleValue;

                    // Read the second double value from the QByteArray
                    double secondDoubleValue;
                    stream.device()->seek(secondDoubleIndex);
                    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
                    stream >> secondDoubleValue;

                    qDebug() << "Float value:" << floatValue;
                    qDebug() << "First double value:" << firstDoubleValue;
                    qDebug() << "Second double value:" << secondDoubleValue;

                    azimuth = (-1)*floatValue;
                    longitude = secondDoubleValue;
                    latitude = firstDoubleValue;

                    emit frameReceived();
                    framesReceived++;
                }
                else
                {
                    qDebug() << "Not enough bytes in the datagram to read a float and two double values";
                }

        }
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

QString Connection::localIpAddress()
{
    QString ipAddress;
    QList<QHostAddress> ipAddresses = QNetworkInterface::allAddresses();

    // Find the first non-local IPv4 address
    for (const QHostAddress &address : ipAddresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
            ipAddress = address.toString();
            break;
        }
    }

    // If we didn't find a non-local IPv4 address, use the localhost address
    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    currentIP = ipAddress;
    qDebug() << ipAddress;
    return ipAddress;
}

QString Connection::byteArrayToBinaryString(const QByteArray& byteArray)
{
    QString binaryString;
    binaryString.reserve(byteArray.size() * 8); // Reserve enough space for the binary string
    char val = 1;
    for (int i = 0; i < byteArray.size(); i++)
    {
        val=1;
        for (int j = 0; j < 8; j++)
        {
            bool bit = byteArray[i] & val;
            val = (val << 1) ;
            binaryString.append(bit == 1 ? '1' : '0');
        }
        binaryString.append(' '); // Add a space between each byte
    }

    return binaryString;
}
