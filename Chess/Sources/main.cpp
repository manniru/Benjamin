#include "channel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *root = engine.rootObjects().first();

    Channel *dbusChnl = new Channel(root);

    QObject::connect(root, SIGNAL(eKeyPressed(int)), dbusChnl, SLOT(keyPressed(int)));

    return app.exec();
}

