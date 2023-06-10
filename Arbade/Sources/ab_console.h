#ifndef AB_CONSOLE_H
#define AB_CONSOLE_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "ab_console_handle.h"

class AbConsole : public QObject
{
    Q_OBJECT
public:
    explicit AbConsole(QObject *ui, QObject *parent = nullptr);
    ~AbConsole();

    void wsl_run(QString cmd);
    void run(QString cmd);

    QString prompt;
    QVector<QString> commands;

public slots:
    void startConsole(QString wsl_path);
    void readyData(QString data, int mode);

signals:
    void startRead();
    void finished();
    void trainFailed();

private:
    void processLine(QString line);
    void CreateCmdProcess();

    HANDLE proc_in_h  = NULL;
    HANDLE proc_out_h = NULL;
    HANDLE proc_err_h = NULL;
    HANDLE handle_in  = NULL;
    PROCESS_INFORMATION piProcInfo;

    AbConsoleHandle *std_err;
    QThread *err_thread;
    AbConsoleHandle *std_out;
    QThread *out_thread;

    int is_ready;
    int init_shit; //flag to skip printing cmd initial welcome message


    QObject   *root;    // root qml object
    QObject   *console_qml; // console qml object
};

#endif // AB_CONSOLE_H
