#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include "ab_scene.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<AbScene>("OpenGLUnderQML", 1, 0, "AbScene");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *mainItem = engine.rootObjects().first();
    ab_setUi(mainItem);

    return app.exec();
}
