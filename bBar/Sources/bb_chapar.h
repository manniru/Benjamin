#ifndef BB_CHAPAR_H
#define BB_CHAPAR_H

#include <QObject>
#include "bb_bar.h"

class BbChapar : public QObject
{
    Q_OBJECT
public:
    explicit BbChapar(QObject *root, QObject *parent = nullptr);

private:
    BbBar *bar;
};

#endif // BB_CHAPAR_H
