#include <QGuiApplication>
#include "enn_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    EnnChapar chapar;
    QString word = "arch";
    chapar.createEnn(word);

    return app.exec();
}
