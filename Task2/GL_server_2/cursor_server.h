#ifndef CURSOR_SERVER_H
#define CURSOR_SERVER_H

#include <QTcpServer>

class CursorServer : public QTcpServer
{
    Q_OBJECT

public:
    CursorServer(QObject *parent = nullptr);

signals:
    void newClientConnected(int clientId);
    void giveNewCoordinates(int clientId, const QString &message);
    void clientDisconnected(int clientId);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    int idClientGiver;
};

#endif
