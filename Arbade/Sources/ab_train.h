#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
#include <QThread>
#include <QProcess>
#include "ab_init_wsl.h"
#include "ab_console_reader.h"

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
    void writeConsole();
    void writeEConsole();
    void WriteToPipe(QString cmd);

private:
    void createKalB();
    void processLine(QString line);

    int state = 0;

    int openApp();
    void CreateChildProcess(QString cmd);

    QObject   *root; // root qml object
    QObject   *wsl_dialog;
    QObject   *console;
    AbInitWSL *wsl;
    QString    wsl_path;

    QThread   *con_thread;
    QProcess *con_read;
    QThread         *err_thread;
    AbConsoleReader *err_read;

    HANDLE h_in_read = NULL;
    HANDLE h_in_write = NULL;
    HANDLE h_out_write = NULL;
    HANDLE g_hInputFile = NULL;
};


#endif // AB_TRAIN_H
