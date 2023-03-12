#ifndef AB_TRAIN_H
#define AB_TRAIN_H

#include <QObject>
#include "ab_init_wsl.h"

class AbTrain : public QObject
{
    Q_OBJECT
public:
    explicit AbTrain(QObject *ui, QObject *parent = nullptr);
    ~AbTrain();

private slots:
    void processKey(int key);

private:
    void initWsl();

    QObject* root;//root qml object
    AbInitWSL *wsl;
};

#endif // AB_TRAIN_H
