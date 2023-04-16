#ifndef AB_CONSOLE_H
#define AB_CONSOLE_H

#include <QObject>
#include <windows.h>

#define CONSOLE_BUF_SIZE 4096

#define AB_CONSOLE_NORML 0
#define AB_CONSOLE_ERROR 1
#define AB_CONSOLE_INPUT 2

class AbConsole : public QObject
{
    Q_OBJECT
public:
    explicit AbConsole(QObject *parent = nullptr);
    ~AbConsole();

    void wsl_run(QString cmd);
    void run(QString cmd);

    HANDLE handle = NULL;
    HANDLE handle_err = NULL; // implement error in future

public slots:
    void readData();
    void startConsole(QString wsl_path);

signals:
    void readyData(QString data, int mode);
    void finished();

private:
    void processLine(QString line);
    void CreateCmdProcess();

    HANDLE h_in_read = NULL;
    HANDLE h_in_write = NULL;
    PROCESS_INFORMATION piProcInfo;

    int is_ready;
    QString prompt;
    QVector<QString> commands;
};

#endif // AB_CONSOLE_H
