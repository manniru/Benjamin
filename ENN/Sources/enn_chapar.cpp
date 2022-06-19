#include "enn_chapar.h"
#include "enn_test.h"

EnnChapar::EnnChapar(int mode, float l_rate)
{
    if( mode==ENN_LEARN_MODE )
    {
        learnMode(l_rate);
    }
    else
    {
        testMode();
    }
}

EnnChapar::~EnnChapar()
{

}

void EnnChapar::learnMode(float l_rate)
{
    QStringList word_list = enn_listDirs(ENN_TRAIN_DIR);

    int len = word_list.size();
    for( int i=0 ; i<len ; i++ )
    {
        qDebug() << word_list[i];
        EnnNetwork net(word_list[i]);
        net.train(l_rate);
    }
}

void EnnChapar::testMode()
{
    QString model_name = "echo";
    QString input_name = "five" ;
    EnnNetwork net(model_name);
    net.load();
    EnnTest    in(input_name);
    vec_t out = net.test(&(in.images));
    qDebug() << "Load:" << in.img_address;
    qDebug() << "Test Mode on Model:" << model_name
             << "Input:" << input_name
             << out[0] << out[1];
}
