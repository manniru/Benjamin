#include "enn_chapar.h"
#include "enn_test.h"

EnnChapar::EnnChapar(QObject *ui, EnnCmdOptions *options,
                     QObject *parent) : QObject(parent)
{
    root = ui;
    scene = new EnnScene(root);
    return;

    if( options->mode==ENN_LEARN_MODE )
    {
        if( options->word.length() )
        {
            singleMode(options->learning_rate, options->word);
        }
        else
        {
            learnMode(options->learning_rate);
        }
    }
    else if( options->mode==ENN_TEST_MODE )
    {
        testMode();
    }
    else if( options->mode==ENN_TF_MODE )
    {
        testFullMode();
    }
    else if( options->mode==ENN_FILE_MODE )
    {
        fileMode();
    }
}

EnnChapar::~EnnChapar()
{

}

void EnnChapar::learnMode(float l_rate)
{
    QStringList word_list = bt_parseLexicon(BT_WORDS_PATH);
    while( 1 ) // until all model reached target loss
    {
        int learned_count = 0;

        int len = word_list.size();

        for( int i=0 ; i<len ; i++ )
        {
            qDebug() << word_list[i];
            EnnNetwork net(word_list[i], i);
            float first_loss = net.last_loss;
            net.train(l_rate);
            float diff_loss = net.last_loss - first_loss;

            if( net.last_loss<ENN_TARGET_LOSS )
            {
                learned_count++;
            }
            if( diff_loss>0 ) // we are not learning anything
            {                 // after 100 epoch
                learned_count++;
                qDebug() << "BAD LEARNING" << diff_loss;
            }
            printf("----- learned [%d/%d] word=%s loss=%.2f ----\n",
                   learned_count, len, word_list[i].toStdString().c_str(),
                   net.last_loss);
        }

        if( learned_count>=len )
        {
            break;
        }
    }
}

void EnnChapar::singleMode(float l_rate, QString l_word)
{
    QStringList word_list = bt_parseLexicon(BT_WORDS_PATH);
    float learn_rate = l_rate;
    while( 1 ) // until all model reached target loss
    {
        int index = word_list.indexOf(l_word);
        if( index==-1 )
        {
            qDebug() << "Error 101: word" << l_word
                     << "does not exist in lexicon";
            qDebug() << "Error 101: word" << word_list;
            return;
        }

        qDebug() << index << word_list[index];
        EnnNetwork net(word_list[index], index);
        float first_loss = net.last_loss;
        qDebug() << "Loaded Loss" << first_loss;
        net.train(learn_rate);
        float diff_loss = net.last_loss - first_loss;
        if( diff_loss>0 ) // we are not learning anything
        {                 // after 100 epoch
            qDebug() << "BAD LEARNING" << -diff_loss
                     << "imporoved";
            return;
        }
        if( std::isnan(net.last_loss) )
        {
            learn_rate /= 1.2;
        }
        printf("----- learned word=%s loss=%.2f ----\n",
               word_list[index].toStdString().c_str(),
               net.last_loss);

        if( net.last_loss<ENN_TARGET_LOSS )
        {
            return;
        }
    }
}

void EnnChapar::testMode()
{
    QString model_name = "echo";
    QString input_name = "five" ;
    EnnNetwork net(model_name, -1); //id -1 works all the time
//    net.load();
    EnnTest    in(input_name);
    vec_t out = net.test(&(in.image));
    qDebug() << "Load:" << in.img_address;
    qDebug() << "Test Mode on Model:" << model_name
             << "Input:" << input_name
             << "O0" << out[0]
             << "O1" << out[1];

    net.benchmark();
}

void EnnChapar::testFullMode()
{
    setbuf(stdout,NULL);

    QString enn_dir = ab_getAudioPath() + "enn";
    enn_dir += QDir::separator();
    QStringList word_list = enn_listDirs(enn_dir);

    int len = word_list.size();
    for( int i=0 ; i<len ; i++ )
    {
        //id -1 works all the time
        EnnNetwork net(word_list[i], -1);
        net.load();

        int test_len = net.dataset->false_datas.size();

        float loss = 0;
        int  wrong = 0;
        for( int j=0 ; j<test_len ; j++ )
        {
            vec_t out = net.test(&(net.dataset->false_datas[j]));
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

void EnnChapar::fileMode()
{
    setbuf(stdout,NULL);

    qDebug() << "arch";
    EnnDataset dataset("arch", true);
}
