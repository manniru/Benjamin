#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include "ab_scene.h"
#include "ab_train.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
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
    else if( QDir(wsl_path+"\\Benjamin\\Tools").exists()==0 )
    {
        train.initWsl();
    }

    return app.exec();
}
