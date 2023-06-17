#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
#include <QThread>
#include "ab_init_wsl.h"
#include "ab_console.h"
#include "ab_stat.h"

#define AB_MODEL_DIR    "..\\Tools\\Model"
#define AB_TEST_PERCENT 3.0

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
    void handleTrainError();
    void createENN();
    void copyEFalseToOnline();
    void trainENN();
    void trainEnnWord(int word_id);
    void genEFinished();

private:
    void train();
    void checkModelExist();
    void addTestSample(int count);
    int  needTestCount();
    int  getTestCount();
    int  getTrainCount();
    void removeEmptyDirs();
    void checkOnlineExist();
    void updateWerSer();
    void checkBenjamin();
    int checkEFalseDir();

    QObject   *root;            // root qml object
    QObject   *topbar;          // topbar qml object
    QObject   *wsl_dialog;
    QObject   *console_qml;     // console qml object
    QObject   *efalse_dialog;     // verify efalse dialog qml object
    QObject   *train_enn_qml;   // Train Enn Dialog qml object
    QObject   *message; // message qml object

    QThread   *wsl_thread;
    AbInitWSL *wsl;

    QThread   *con_thread;
    AbConsole *console;
    QThread   *enn_thread;
    AbConsole *enn_console;
    AbStat    *stat;
    QString    enn_word;
    QString    wsl_path;
    int        train_failed;
    int        init_flag;
};

#endif // AB_TRAIN_H
