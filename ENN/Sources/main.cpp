#include <QGuiApplication>
#include "enn_chapar.h"

int main(int argc, char *argv[])
{
    float learning_rate = ENN_LEARN_RATE;
    int   mode = ENN_LEARN_MODE;
    if( argc>1 )
    {
        QString arg1 = QString(argv[1]);

        if( arg1=="t" )
        {
            mode = ENN_TEST_MODE;
        }
        else if( arg1=="tf" )
        {
            mode = ENN_TF_MODE;
        }
        else if( arg1=="f" )
        {
            mode = ENN_FILE_MODE;
        }
        else
        {
            learning_rate = arg1.toFloat();
        }
    }
    EnnChapar chapar(mode, learning_rate);

    return 0;
}
