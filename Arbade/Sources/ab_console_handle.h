#ifndef AB_CONSOLE_HANDLE_H
#define AB_CONSOLE_HANDLE_H

#include <QObject>
#include <windows.h>

#define AB_CONSOLE_NORML 0
#define AB_CONSOLE_ERROR 1

#define CONSOLE_BUF_SIZE 4096

class AbConsoleHandle: public QObject
{
    Q_OBJECT
public:
    explicit AbConsoleHandle(int fl, QObject *parent = nullptr);
    ~AbConsoleHandle();

    HANDLE handle = NULL;
public slots:
    void readData();

signals:
    void readyData(QString data, int mode);

private:
    int flag;
};

#endif // AB_CONSOLE_HANDLE_H
