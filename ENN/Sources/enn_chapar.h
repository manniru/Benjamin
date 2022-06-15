#ifndef ENN_CHAPAR_H
#define ENN_CHAPAR_H

#include <QString>
#include <QDir>
#include <QtDebug>

#include <tiny_dnn/tiny_dnn.h>

#define ENN_FALSE_COUNT 2
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
    void parseImagesT(QString path, QString word);
    void parseImagesF(QString path, QString word);
    QStringList listImages(QString path, int num=-1);
    QStringList listDirs(QString path);
    void epochLog();

    network<sequential> net;
    vector<vec_t> train_labels;
    vector<vec_t>   train_images;
    vector<vec_t> test_labels;
    vector<vec_t>   test_images;
};

#endif // ENN_CHAPAR_H
