#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QMap>
#include <QDateTime>
#include "cursor_server.h"

class Dialog : public QWidget
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);

private slots:
    void updateClientInfo(int clientId, const QString &message);
    void updateClientStatus(int clientId);
    void removeDisconectedClient(int clientId);

private:
    QLabel *statusLabel;
    QPushButton *quitButton;
    QTableView *tableView;
    QStandardItemModel *model;
    CursorServer server;
    QMap<int, QPair<QString, QString>> clientInfoMap;
    QMap<int, qint64> clientActivityMap;
};

#endif
