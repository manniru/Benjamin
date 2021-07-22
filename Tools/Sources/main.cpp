#include <QGuiApplication>
#include "bt_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    int isNative = 1;
    ReChapar *chaper = new ReChapar;

    return app.exec();
}

