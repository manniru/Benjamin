#ifndef ENN_CHAPAR_H
#define ENN_CHAPAR_H

#include <QGuiApplication>
#include "backend.h"
#include "enn_network.h"
#include "enn_scene.h"
#include "enn_sample_link.h"

#define ENN_LEARN_MODE 1
#define ENN_TEST_MODE  2
#define ENN_TF_MODE    3
#define ENN_FILE_MODE  4
#define ENN_UI_MODE    5

typedef struct EnnCmdOptions
{
    int mode;
    float learning_rate;
    QString word;
}EnnCmdOptions;

class EnnChapar : public QObject
{
    Q_OBJECT
public:
    explicit EnnChapar(QGuiApplication *app, EnnCmdOptions *options,
                       QObject *parent = nullptr);
    ~EnnChapar();

    void testMode();
    void fileMode();
    void testFullMode();
    void learnMode(float l_rate);
    void singleMode(float l_rate, QString l_word);

private:
    EnnScene *scene;
    QObject *root;       // root qml object
    QObject *sample_viewer;   //rec list qml object
};

#endif // ENenn_listDirs_H
