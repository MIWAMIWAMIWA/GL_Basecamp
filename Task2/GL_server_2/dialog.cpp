#include "dialog.h"
#include "cursor_server.h"
#include <QtWidgets>
#include <QtNetwork>

Dialog::Dialog(QWidget *parent)
    : QWidget(parent)
{
    resize(320, height());
    statusLabel = new QLabel;
    statusLabel->setWordWrap(true);
    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);

    if (!server.listen(QHostAddress::Any, 5577)) {
        QMessageBox::critical(this, tr("Server for cursor clients"),
                              tr("Unable to start the server: %1.")
                                  .arg(server.errorString()));
        close();
        return;
    }

    QString ipAddress;
    const QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (const QHostAddress &entry : ipAddressesList) {
        if (entry != QHostAddress::LocalHost && entry.toIPv4Address()) {
            ipAddress = entry.toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\n")
                             .arg(ipAddress)
                             .arg(server.serverPort()));

    connect(quitButton, &QPushButton::clicked, this, &Dialog::close);

    model = new QStandardItemModel(0, 3, this);
    model->setHeaderData(0, Qt::Horizontal, tr("Client ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Activity Status"));
    model->setHeaderData(2, Qt::Horizontal, tr("Connection Status"));

    tableView = new QTableView;
    tableView->setModel(model);
    tableView->setColumnWidth(0, 60);
    tableView->setColumnWidth(1, 100);
    tableView->setColumnWidth(2, 120);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
    setWindowTitle(tr("server for cursor clients"));

    connect(&server, &CursorServer::newClientConnected, this, [this](int clientId) {
        updateClientInfo(clientId, QString());
    });
    connect(&server, &CursorServer::giveNewCoordinates, this, &Dialog::updateClientInfo);
    connect(&server, &CursorServer::clientDisconnected, this, &Dialog::updateClientStatus);
}

void Dialog::updateClientInfo(int clientId, const QString &message)
{
    QString status = clientInfoMap.contains(clientId) && clientInfoMap[clientId].first == message ? tr("Passive") : tr("Active");
    clientInfoMap[clientId] = qMakePair(message, status);
    clientActivityMap[clientId] = QDateTime::currentSecsSinceEpoch();

    model->removeRows(0, model->rowCount());

    for (auto it = clientInfoMap.begin(); it != clientInfoMap.end(); ++it) {
        QList<QStandardItem *> items;
        items.append(new QStandardItem(QString::number(it.key())));
        items.append(new QStandardItem(it.value().second));
        items.append(new QStandardItem(tr("Connected")));
        model->appendRow(items);
    }
}

void Dialog::updateClientStatus(int clientId)
{
    if (clientInfoMap.contains(clientId)) {
        clientInfoMap[clientId].second = tr("Disconnected");
        clientActivityMap[clientId] = QDateTime::currentSecsSinceEpoch();
    }

    model->removeRows(0, model->rowCount()); // Clear the table

    for (auto it = clientInfoMap.begin(); it != clientInfoMap.end(); ++it) {
        QList<QStandardItem *> items;
        items.append(new QStandardItem(QString::number(it.key())));
        items.append(new QStandardItem(it.value().second));
        items.append(new QStandardItem(it.value().second == tr("Disconnected") ? tr("Disconnected") : tr("Connected")));
        model->appendRow(items);
    }

    QTimer::singleShot(5000, this, [this, clientId]() { removeDisconectedClient(clientId); });
}

void Dialog::removeDisconectedClient(int clientId)
{
    if (clientInfoMap.contains(clientId) && clientInfoMap[clientId].second == tr("Disconnected")) {
        clientInfoMap.remove(clientId);
        clientActivityMap.remove(clientId);
    }

    model->removeRows(0, model->rowCount());

    for (auto it = clientInfoMap.begin(); it != clientInfoMap.end(); ++it) {
        QList<QStandardItem *> items;
        items.append(new QStandardItem(QString::number(it.key())));
        items.append(new QStandardItem(it.value().second));
        items.append(new QStandardItem(it.value().second == tr("Disconnected") ? tr("Disconnected") : tr("Connected")));
        model->appendRow(items);
    }
}
