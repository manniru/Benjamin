#include "enn_chapar.h"
#include "enn_test.h"

EnnChapar::EnnChapar(int mode, float l_rate)
{
    if( mode==ENN_LEARN_MODE )
    {
        learnMode(l_rate);
    }
    else if( mode==ENN_TEST_MODE )
    {
        testMode();
    }
    else if( mode==ENN_TF_MODE )
    {
        testFullMode();
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

void EnnChapar::testFullMode()
{
    setbuf(stdout,NULL);
    QStringList word_list = enn_listDirs(ENN_TRAIN_DIR);

    int len = word_list.size();
    for( int i=0 ; i<len ; i++ )
    {
        EnnNetwork net(word_list[i]);
        net.load();

        int test_len = net.dataset->false_images.size();

        float loss = 0;
        int  wrong = 0;
        for( int j=0 ; j<test_len ; j++ )
        {
            vec_t out = net.test(&(net.dataset->false_images[j]));
            loss += out[1];

            if( out[0]<0.6 )
            {
                wrong++;
            }
        }
        printf("%5.5s loss:%.2f wrong:[%d/%d]\n",
               word_list[i].toStdString().c_str(),
               loss, wrong, test_len);
    }
}
