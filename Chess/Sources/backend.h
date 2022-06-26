#ifndef BACKEND_H
#define BACKEND_H

#include <gio/gio.h>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlProperty>
#include <QtDBus>

struct screen_pos{
    int x;
    int y;
};

int getIntCommand(char *command);
QString getStrCommand(QString *command);
screen_pos getPrimaryScreen();
void updateScreenInfo(QObject *item);
void showNotif(QObject *item);
QString getTranslateOnline(QString word);

#endif // BACKEND_H
