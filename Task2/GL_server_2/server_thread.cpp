#include "server_thread.h"

ServerThread::ServerThread(qintptr socketDescriptor, int clientId, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor), clientId(clientId)
{
}

void ServerThread::run()
{
    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(socketDescriptor)) {
        emit error(tcpSocket.error());
        return;
    }

    connect(&tcpSocket, &QTcpSocket::readyRead, this, [this, &tcpSocket]() {
        QDataStream in(&tcpSocket);
        in.setVersion(QDataStream::Qt_6_5);
        QString coordinates;
        in >> coordinates;
        emit newCoordinates(clientId, coordinates);
    });

    connect(&tcpSocket, &QTcpSocket::disconnected, this, [this, &tcpSocket]() {
        emit clientDisconnected(clientId); //make it doesnt delete but wait
        tcpSocket.deleteLater();
        quit();
    });

    exec();
}
