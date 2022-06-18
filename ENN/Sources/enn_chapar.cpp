#include "enn_chapar.h"

EnnChapar::EnnChapar(float l_rate)
{
    QStringList word_list = enn_listDirs(ENN_TRAIN_DIR);

    int len = word_list.size();
    for( int i=0 ; i<len ; i++ )
    {
        qDebug() << "#######" << word_list[i]
                 << "######";
        EnnNetwork chapar(word_list[i]);
        chapar.train(l_rate);
    }
}

EnnChapar::~EnnChapar()
{

}
