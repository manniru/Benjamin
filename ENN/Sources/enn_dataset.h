#ifndef ENN_PARSER_H
#define ENN_PARSER_H

#include <QString>
#include <QDir>
#include <QDebug>

#include <tiny_dnn/tiny_dnn.h>
#include "config.h"
#include "backend.h"

using namespace tiny_dnn;

class EnnDataset
{
public:
    EnnDataset(QString word); // binary word to classify
    ~EnnDataset();

    std::vector<vec_t>   train_images;
    std::vector<label_t> train_labels;
    std::vector<vec_t>   train_labels_v;
    QStringList          train_path;
    std::vector<vec_t>   test_images;
    std::vector<label_t> test_labels;
    std::vector<vec_t>   test_labels_v;
    QStringList          test_path;

    QString m_name; //model name
private:
    void parseImagesT(QString path);
    void parseImagesF(QString path);
    void shuffleTest(std::mt19937 *eng1, std::mt19937 *eng2);
    void shuffleData();

    QStringList listImages(QString path, int num=-1);
};

#endif // ENenn_listDirs_H
