#ifndef FORTUNETHREAD_H
#define FORTUNETHREAD_H

#include <QThread>
#include <QTcpSocket>

class ServerThread : public QThread
{
    Q_OBJECT

public:
    ServerThread(qintptr socketDescriptor, int clientId, QObject *parent = nullptr);
    void run() override;

signals:
    void error(QTcpSocket::SocketError socketError);
    void newCoordinates(int clientId, const QString &message);
    void clientDisconnected(int clientId);

private:
    qintptr socketDescriptor;
    int clientId;
};

#endif
