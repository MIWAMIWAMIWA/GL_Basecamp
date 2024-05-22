#ifndef CURSOR_CLIENT_H
#define CURSOR_CLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QTimer>
#include <QPoint>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE

class CursorClient : public QWidget
{
    Q_OBJECT

public:
    CursorClient(QWidget *parent = nullptr);

private slots:
    void connectToServer();
    void displayError(QAbstractSocket::SocketError socketError);
    void sendCoordinates();
    void onConnected();
    void disconnectFromServer();

private:
    QLabel *statusLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QTcpSocket *tcpSocket;
    QTimer sendTimer;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
};

#endif // CURSOR_CLIENT_H
