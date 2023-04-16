#include <QGuiApplication>
#include "enn_chapar.h"
#include "../PNN/aj_dllgen.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
#ifdef WIN32
    aj_dllGen();
#endif

    float learning_rate = ENN_LEARN_RATE;
    int   mode = ENN_LEARN_MODE;
    if( argc>1 )
    {
        QString arg1 = QString(argv[1]);

        if( arg1=="t" )
        {
            mode = ENN_TEST_MODE;
        }
        else if( arg1=="tf" ) // test full mode
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
