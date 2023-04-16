#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
#include <QThread>
#include "ab_init_wsl.h"
#include "ab_console.h"

#define AB_MODEL_DIR "..\\Tools\\Model2"

class AbTrain : public QObject
{
    Q_OBJECT
public:
    explicit AbTrain(QObject *ui, QObject *parent = nullptr);
    ~AbTrain();

    void initWsl();

    AbInitWSL *wsl;

signals:
    void readConsole();
    void readError();
    void startConsole(QString wsl_path);

private slots:
    void processKey(int key);
    void writeToQml(QString line, int flag=0);
    void trainFinished();

private:
    void initKalB();
    void checkModelExist();

    QObject   *root;    // root qml object
    QObject   *wsl_dialog;
    QObject   *console_qml; // console qml object
    QString    wsl_path;

    QThread   *con_thread;
    AbConsole *console;
    QThread   *err_thread;
    AbConsole *err_read;

    int init_flag;
};


#endif // AB_TRAIN_H
