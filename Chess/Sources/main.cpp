#include "ch_channel_l.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *root = engine.rootObjects().first();

    ChChannelL *dbusChnl = new ChChannelL(root);

    QObject::connect(root, SIGNAL(eKeyPressed(int)), dbusChnl, SLOT(keyPressed(int)));

    return app.exec();
}

