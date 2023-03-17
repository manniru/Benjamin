#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
//#include <tchar.h>
//#include <strsafe.h>
#include "ab_init_wsl.h"

#define CONSOLE_BUF_SIZE 4096

class AbTrain : public QObject
{
    Q_OBJECT
public:
    explicit AbTrain(QObject *ui, QObject *parent = nullptr);
    ~AbTrain();

private slots:
    void processKey(int key);

private:
    void initWsl();
    void createKalB();
    void writeConsole(QString line);

    int openApp();
    void CreateChildProcess(QString cmd);
    void WriteToPipe();
    void ReadFromPipe();

    QObject   *root; // root qml object
    QObject   *wsl_dialog;
    QObject   *console;
    AbInitWSL *wsl;
    QString    wsl_path;

    HANDLE h_in_read = NULL;
    HANDLE h_in_write = NULL;
    HANDLE h_out_read = NULL;
    HANDLE h_out_write = NULL;
    HANDLE g_hInputFile = NULL;
};


#endif // AB_TRAIN_H
