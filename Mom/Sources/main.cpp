#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlProperty>
#include <QDebug>

#include "mm_chapar.h"
#include "mm_watcher.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);
    app.setOrganizationName("WBT");
    app.setOrganizationDomain("WBT.com");
    app.setApplicationName("PolyBar");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
    QObject *root = engine.rootObjects().first();

    MmChapar  chapar(root);
    MmWatcher watcher;

    chapar.key->e_key->altTab();

    return app.exec();
}
