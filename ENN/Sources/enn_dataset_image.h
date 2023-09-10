#ifndef ENN_PARSER_H
#define ENN_PARSER_H

#include <QString>
#include <QDir>
#include <QDebug>

#include <tiny_dnn/tiny_dnn.h>
#include "config.h"
#include "backend.h"

#ifdef ENN_IMAGE_DATASET

using namespace tiny_dnn;

class EnnDataset
{
public:
    EnnDataset(QString word, int test=false); // binary word to classify
    ~EnnDataset();

    std::vector<vec_t>   train_datas;
    std::vector<int> train_labels;
    std::vector<QString> train_path; //for debug
    std::vector<vec_t>   test_datas;
    std::vector<int> test_labels;
    std::vector<vec_t>   false_datas;

    QString m_name; //model name
private:
    void parseImagesT(QString path);
    void parseImagesF(QString path);
    void addImagesT(QString path, int i);
    void addImagesF(QString path, int i, int j);
    void shuffleTest(std::mt19937 *eng1, std::mt19937 *eng2);
    void testFile(QString path);
    void shuffleData();

    int train_size;
};

#endif // ENN_IMAGE_DATASET

#endif // ENN_PARSER_H
