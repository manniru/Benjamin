#include "bt_chapar.h"
#include <QDebug>
#include <QDir>
#include <QThread>
#include <time.h>
#include <stdlib.h>

BtChapar::BtChapar(QObject *parent) : QObject(parent)
{
    gui_len = 3;
    srand(time(NULL));
    int sample_count = gui_len*BT_REC_RATE;
    rec = new BtRecorder(sample_count);
    wav = new BtWavWriter(rec->cy_buf, sample_count);
    connect(rec, SIGNAL(finished()), this, SLOT(writeWav()));
    lexicon = bt_parseLexicon(BT_WORDS_PATH);
}

void BtChapar::writeWav()
{
    wav->write(words);
    words.clear();
}

void BtChapar::record(QString category)
{
    int word_id[AB_WORD_LEN];
    int lexicon_size = lexicon.length();
    for( int i=0 ; i<AB_WORD_LEN ; i++ )
    {
        word_id[i] = rand()%lexicon_size;
    }

    words.resize(AB_WORD_LEN);
    for( int i=0 ; i<AB_WORD_LEN ; i++ )
    {
        words[i].word_id = word_id[i];
        words[i].word = lexicon[word_id[i]];
    }

    wav->setCategory(category);
    rec->startStream();
}

BtChapar::~BtChapar()
{
}
