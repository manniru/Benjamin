#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include "ab_scene.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Binaee");
    app.setOrganizationDomain("Binaee.com");
    app.setApplicationName("ArBade");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *mainItem = engine.rootObjects().first();
    AbScene scene(mainItem);

    return app.exec();
}
