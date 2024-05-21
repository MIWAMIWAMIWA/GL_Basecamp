#include "cursor_server.h"
#include "server_thread.h"

CursorServer::CursorServer(QObject *parent)
    : QTcpServer(parent), idClientGiver(0)
{
}

void CursorServer::incomingConnection(qintptr socketDescriptor)
{
    int clientId = idClientGiver++;
    emit newClientConnected(clientId);

    ServerThread *thread = new ServerThread(socketDescriptor, clientId, this);

    connect(thread, &ServerThread::newCoordinates, this, &CursorServer::giveNewCoordinates);
    connect(thread, &ServerThread::clientDisconnected, this, &CursorServer::clientDisconnected);
    connect(thread, &ServerThread::finished, thread, &QObject::deleteLater);

    thread->start();
}
