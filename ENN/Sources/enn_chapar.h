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

#define O true
#define X false

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
    void minibatchLog();

    // clang-format off
    bool tbl[96] = {
      O, X, X, X, O, O, O, X, X, O, O, O, O, X, O, O,
      O, O, X, X, X, O, O, O, X, X, O, O, O, O, X, O,
      O, O, O, X, X, X, O, O, O, X, X, O, X, O, O, O,
      X, O, O, O, X, X, O, O, O, O, X, X, O, X, O, O,
      X, X, O, O, O, X, X, O, O, O, O, X, O, O, X, O,
      X, X, X, O, O, O, X, X, O, O, O, O, X, O, O, O };
    network<sequential> net;
    progress_display *disp;
    vector<label_t> train_labels;
    vector<vec_t> train_images;
    vector<label_t> test_labels;
    vector<vec_t> test_images;
    int n_minibatch;
    int n_train_epochs;
};

#endif // ENN_CHAPAR_H
