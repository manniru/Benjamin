#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include "ab_scene.h"
#include "ab_train.h"
#include "../PNN/aj_dllgen.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
#ifdef WIN32
    aj_dllGen();
#endif

    app.setOrganizationName("Binaee");
    app.setOrganizationDomain("Binaee.com");
    app.setApplicationName("ArBade");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *root = engine.rootObjects().first();
    AbScene scene(root);
    AbTrain train(scene.editor->stat, root);
    QString wsl_path = ab_getWslPath();
    if( wsl_path.isEmpty() )
    {
        QMetaObject::invokeMethod(root, "initWsl");
    }
    train.initWsl();

    return app.exec();
}
