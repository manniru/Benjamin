#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlProperty>
#include <QDebug>
#include <QScreen>

#include "bb_chapar.h"

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
    QObject *mainItem = engine.rootObjects().first();

    // Set screen width
    QScreen *screen = QGuiApplication::primaryScreen();
    QQmlProperty::write(mainItem, "width", screen->geometry().width());

    BbChapar bar(mainItem);

    return app.exec();
}
