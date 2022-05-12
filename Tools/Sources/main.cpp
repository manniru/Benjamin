#include <QGuiApplication>
#include "bt_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    ReChapar chaper;

    return app.exec();
}
