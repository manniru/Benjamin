#include <QGuiApplication>
#include "bt_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    BtChapar chaper;
    chaper.record("sag");

    return app.exec();
}
