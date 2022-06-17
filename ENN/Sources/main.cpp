#include <QGuiApplication>
#include "enn_network.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QString word = "zero";
    EnnNetwork chapar(word);
    chapar.createEnn();

    return app.exec();
}
