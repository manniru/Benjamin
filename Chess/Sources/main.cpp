#include <QApplication>
#include <QQmlApplicationEngine>
#include <QScreen>

#include "ch_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject *root = engine.rootObjects().first();

#ifdef WIN32
    // Get primary screen width, height
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    QQmlProperty::write(root, "width", screen.width()-20);
    QQmlProperty::write(root, "height", screen.height()-CH_TASKBAR_HEIGHT);
    QQmlProperty::write(root, "x", 0);
    QQmlProperty::write(root, "y", CH_TASKBAR_HEIGHT-20);
    int flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
               | Qt::WindowTransparentForInput;
    QQmlProperty::write(root, "flags", flags);
#endif

    ChChapar chapar(root);

    // This is a bug that the ui thread won't start
    // unless you show the qml for a sec

//    QQmlProperty::write(root, "visible", 0);

    return app.exec();
}
