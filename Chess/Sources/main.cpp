#include "ch_channel_l.h"
#include "ch_processor_l.h"
#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *root = engine.rootObjects().first();

    ChChannelL *channel = new ChChannelL;
    ChProcessorL *processor = new ChProcessorL(channel, root);

    QObject::connect(root, SIGNAL(eKeyPressed(int)),
                     processor, SLOT(keyPressed(int)));

    return app.exec();
}

