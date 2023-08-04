#ifndef ENN_PARSER_H
#define ENN_PARSER_H

#include <QString>
#include <QDir>
#include <QDebug>

#include <tiny_dnn/tiny_dnn.h>
#include "config.h"
#include "backend.h"

#ifndef ENN_IMAGE_DATASET

using namespace tiny_dnn;

class EnnDataset
{
public:
    EnnDataset(QString word, int id,
               int test=false); // binary word to classify
    ~EnnDataset();

    std::vector<vec_t>   train_datas;
    std::vector<label_t> train_labels;
    std::vector<QString> train_path; //for debug
    std::vector<vec_t>   test_datas;
    std::vector<label_t> test_labels;
    std::vector<vec_t>   false_datas;

    QString m_name; //model name
    int     model_id; //model word id

private:
    void createFTest();
    void parseTrues(QString path);
    void parseFalses(QString path);
    bool isSuperFalse(QString filename);
    void addDataT(QString path, int i);
    void addDataF(QString path, int i, int j);
    void testFile(QString path);
    void shuffleData();

    int true_counter;
    int false_counter;
    int train_size;
};

#endif // ENN_IMAGE_DATASET

#endif // ENN_PARSER_H
