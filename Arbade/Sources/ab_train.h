#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
#include <QThread>
#include "ab_init_wsl.h"
#include "ab_console_controller.h"

class AbTrain : public QObject
{
    Q_OBJECT
public:
    explicit AbTrain(QObject *ui, QObject *parent = nullptr);
    ~AbTrain();

    void initWsl();

signals:
    void readConsole();
    void readError();

private slots:
    void processKey(int key);
    void writeConsole(QString line, int flag=0);
    void WriteToPipe(QString cmd);

private:
    void initKalB();

    int openConsole();
    void CreateChildProcess(QString cmd);

    QObject   *root; // root qml object
    QObject   *wsl_dialog;
    QObject   *console;
    AbInitWSL *wsl;
    QString    wsl_path;

    QThread   *con_thread;
    AbConsoleController *con_read;
    QThread         *err_thread;
    AbConsoleController *err_read;

    HANDLE h_in_read = NULL;
    HANDLE h_in_write = NULL;
    PROCESS_INFORMATION piProcInfo;
    int init_flag;
};


#endif // AB_TRAIN_H
