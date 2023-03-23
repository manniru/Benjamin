#ifndef AB_CONSOLE_CONTROLLER_H
#define AB_CONSOLE_CONTROLLER_H

#include <QObject>
#include <windows.h>

#define CONSOLE_BUF_SIZE 4096

#define AB_CONSOLE_NORML 0
#define AB_CONSOLE_ERROR 1
#define AB_CONSOLE_INPUT 2

class AbConsoleController : public QObject
{
    Q_OBJECT
public:
    explicit AbConsoleController(int mode, QObject *parent = nullptr);
    ~AbConsoleController();

    HANDLE handle = NULL;

public slots:
    void run();

signals:
    void readyData(QString data, int mode);
    void sendCommand(QString cmd);

private:
    void processLine(QString line);

    QStringList commands;
    int line_number = 0;
    int flag;
};

#endif // AB_CONSOLE_CONTROLLER_H
