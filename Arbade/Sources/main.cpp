#include <QGuiApplication>
#include "ab_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    AbChapar chaper;
    QString category="sag";
    int count = 100;

    if( argc>1 )
    {
        category = argv[1];
    }
    if( argc>2 )
    {
        count = atoi(argv[2]);
    }

    chaper.record(count, category);

    return app.exec();
}
