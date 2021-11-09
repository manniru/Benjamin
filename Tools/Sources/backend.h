#ifndef BACKEND_H
#define BACKEND_H

#include <QtDBus>
#include <QDebug>
#include <stdio.h>
#include <unistd.h>

int getIntCommand(char *command);
QString getStrCommand(QString command);
void printTime(clock_t start);


#endif // BACKEND_H
