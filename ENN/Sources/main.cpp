#include <QGuiApplication>
#include "enn_chapar.h"

int main(int argc, char *argv[])
{
    float learning_rate = ENN_LEARN_RATE;
    if( argc>1 )
    {
        qDebug() << "hi" << argv[1];
        learning_rate = QString(argv[1]).toFloat();
    }
    EnnChapar chapar(learning_rate);

    return 0;
}
