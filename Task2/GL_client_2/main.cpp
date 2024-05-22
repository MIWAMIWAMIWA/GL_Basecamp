#include "cursor_client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CursorClient w;
    w.show();
    return app.exec();
}
