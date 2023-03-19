#ifndef AB_CONSOLE_READER_H
#define AB_CONSOLE_READER_H

#include <QObject>
#include <windows.h>

#define CONSOLE_BUF_SIZE 4096

#define AB_CONSOLE_NORML 0
#define AB_CONSOLE_ERROR 1
#define AB_CONSOLE_INPUT 2

class AbConsoleReader : public QObject
{
    Q_OBJECT
public:
    explicit AbConsoleReader(int mode, QObject *parent = nullptr);
    ~AbConsoleReader();

    HANDLE handle = NULL;

public slots:
    void run();

signals:
    void readyData(QString data, int mode);
    void sendCommand(QString cmd);

private:
    void processLine(QString line);

    int state = 0;
    int flag;
};

#endif // AB_CONSOLE_READER_H
