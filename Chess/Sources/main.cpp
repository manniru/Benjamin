#include <QApplication>
#include <QQmlApplicationEngine>
#include <QScreen>

#include "ch_chapar.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *root = engine.rootObjects().first();

#ifdef WIN32
    // Get primary screen width, height
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    QQmlProperty::write(root, "width", screen.width());
    QQmlProperty::write(root, "height", screen.height()-CH_TASKBAR_HEIGHT);
    QQmlProperty::write(root, "x", 0);
    QQmlProperty::write(root, "y", 0);
    int flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
    QQmlProperty::write(root, "flags", flags);
#endif

    ChChapar *chapar = new ChChapar(root);

    return app.exec();
}

