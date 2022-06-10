#ifndef ENN_CHAPAR_H
#define ENN_CHAPAR_H

#include <QString>
#include <QDir>

#include "tiny_dnn/tiny_dnn.h"

#define ENN_FALSE_COUNT 10
#define ENN_TRAIN_DIR   "../Nato/audio/enn/"

using namespace std;
using namespace tiny_dnn;

class EnnChapar
{
public:
    EnnChapar();
    ~EnnChapar();

    void createEnn(QString word);

private:
    void parseImages(QString path, QString word);
    QStringList listImages(QString path, int num=-1);
    QStringList listDirs(QString path);

    vector<label_t> train_labels;
    vector<vec_t> train_images;
};

#endif // ENN_CHAPAR_H
