#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
#include <QThread>
#include "ab_init_wsl.h"
#include "ab_console.h"
#include "ab_stat.h"

#define AB_MODEL_DIR "..\\Tools\\Model"

class AbTrain : public QObject
{
    Q_OBJECT
public:
    explicit AbTrain(AbStat *st, QObject *ui,
                     QObject *parent = nullptr);
    ~AbTrain();

    void initWsl();

signals:
    void readConsole();
    void readError();
    void createWSL(QString drive);

private slots:
    void processKey(int key);
    void trainFinished();
    void WslCreated();

private:
    void train();
    void trainENN();
    void createENN();
    void checkModelExist();
    void addTestSample(int count);
    int  needTestCount();
    int  getTestCount();
    int  getTrainCount();

    QObject   *root;    // root qml object
    QObject   *wsl_dialog;
    QObject   *console_qml; // console qml object
    QString    wsl_path;

    QThread   *wsl_thread;
    AbInitWSL *wsl;

    QThread   *con_thread;
    AbConsole *console;
    QThread   *enn_thread;
    AbConsole *enn_console;
    AbStat    *stat;
};


#endif // AB_TRAIN_H
