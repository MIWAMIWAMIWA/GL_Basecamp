#include "cursor_client.h"
#include <QtWidgets>
#include <QtNetwork>

CursorClient::CursorClient(QWidget *parent)
    : QWidget(parent), tcpSocket(new QTcpSocket(this))
{
    statusLabel = new QLabel(tr("Disconnected"));

    hostLineEdit = new QLineEdit("192.168.56.1");
    portLineEdit = new QLineEdit("5577");

    connectButton = new QPushButton(tr("Connect"));
    disconnectButton = new QPushButton(tr("Disconnect"));

    QHBoxLayout *hostLayout = new QHBoxLayout;
    hostLayout->addWidget(new QLabel(tr("Host:")));
    hostLayout->addWidget(hostLineEdit);
    hostLayout->addWidget(new QLabel(tr("Port:")));
    hostLayout->addWidget(portLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(hostLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Cursor Client"));

    connect(tcpSocket, &QTcpSocket::connected, this, &CursorClient::onConnected);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &CursorClient::displayError);

    connect(&sendTimer, &QTimer::timeout, this, &CursorClient::sendCoordinates);

    connect(connectButton, &QPushButton::clicked, this, &CursorClient::connectToServer);
    connect(disconnectButton, &QPushButton::clicked, this, &CursorClient::disconnectFromServer);
}

void CursorClient::connectToServer()
{
    tcpSocket->abort();
    tcpSocket->connectToHost(hostLineEdit->text(), portLineEdit->text().toUShort());
    statusLabel->setText(tr("Connecting..."));
}

void CursorClient::disconnectFromServer()
{
    sendTimer.stop();
    tcpSocket->disconnectFromHost();
    statusLabel->setText(tr("Disconnected"));
}

void CursorClient::onConnected()
{
    sendTimer.start(10000);
    statusLabel->setText(tr("Connected"));
}

void CursorClient::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Cursor Client"),
                                 tr("Host not found"));
        statusLabel->setText(tr("Disconnected"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Cursor Client"),
                                 tr("Connection refused"));
        statusLabel->setText(tr("Disconnected"));
        break;
    default:
        QMessageBox::information(this, tr("Cursor Client"),
                                 tr("Error occurred: %1.")
                                     .arg(tcpSocket->errorString()));
        statusLabel->setText(tr("Disconnected"));
    }
}

void CursorClient::sendCoordinates()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QPoint cursorPos = QCursor::pos();
        QString coordinates = QString("(%1, %2)").arg(cursorPos.x()).arg(cursorPos.y());
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_5);
        out << coordinates;
        tcpSocket->write(data);
    }
}

