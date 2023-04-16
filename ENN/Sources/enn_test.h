#ifndef ENN_TEST_H
#define ENN_TEST_H

#include <QString>
#include <QDir>
#include <QDebug>
#include <QImage>

#include <tiny_dnn/tiny_dnn.h>
#include "config.h"
#include "backend.h"

using namespace tiny_dnn;

class EnnTest
{
public:
    EnnTest(QString word); // binary word to classify
    ~EnnTest();

    vec_t   image;

    QString data_dir; //model name
    QString img_address;
private:
    void readFirstSample();

    QStringList listImages(QString path, int num=-1);
};

#endif // ENenn_listDirs_H
