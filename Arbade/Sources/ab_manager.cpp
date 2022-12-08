#include "ab_chapar.h"
#include <QDebug>
#include <QDir>
#include <QThread>
#include <time.h>
#include <stdlib.h>

AbChapar::AbChapar(QObject *parent) : QObject(parent)
{
    gui_len = 3;
    gui_pause = 1000;

    srand(time(NULL));
    int sample_count = gui_len*BT_REC_RATE;
    rec = new AbRecorder(sample_count);
    wav = new AbWavWriter(rec->cy_buf, sample_count);
    connect(rec, SIGNAL(finished()), this, SLOT(writeWav()));
    lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
    readWordList();
}

void AbChapar::writeWav()
{
    wav->write(wav_path);
    r_counter--;
    if( r_counter>0 )
    {
        record(r_counter, wav->category);
    }
}

void AbChapar::record(int count, QString category)
{
    r_counter = count;
    wav_path = getRandPath(category);
    wav->setCategory(category);
    QThread::msleep(gui_pause);
    qDebug() << "start record";
    rec->startStream();
}

QString AbChapar::getRandPath(QString category)
{
    int word_id[AB_WORD_LEN];
    int lexicon_size = lexicon.length();
    QVector<AbWord> words;

    while( true )
    {
        for( int i=0 ; i<AB_WORD_LEN ; i++ )
        {
            word_id[i] = rand()%lexicon_size;
        }

        words.clear();
        words.resize(AB_WORD_LEN);
        for( int i=0 ; i<AB_WORD_LEN ; i++ )
        {
            words[i].word_id = word_id[i];
            words[i].word = lexicon[word_id[i]];
        }

        QString file_name = getFileName(words, category);

        if( QFile::exists(file_name)==0 )
        {
            printWords(words);
            return file_name;
        }
    }

    return "";
}

QString AbChapar::getFileName(QVector<AbWord> words,
                              QString category)
{
    // verified base name
    QString base_name = KAL_AU_DIR"train/";
    base_name += category + "/";
    base_name += wordToId(words);
    QString name = base_name + ".wav";

    return name;
}

void AbChapar::readWordList()
{
    if( word_list.size() )
    {
        return;
    }

    QFile wl_file(BT_WORDLIST_PATH);

    if( !wl_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << BT_WORDLIST_PATH;
        return;
    }

    while ( !wl_file.atEnd() )
    {
        QString line = QString(wl_file.readLine());
        line.remove(QRegExp("\\n")); //remove new line
        if( line.length() )
        {
            word_list.push_back(line);
        }
    }

    wl_file.close();
}

QString AbChapar::wordToId(QVector<AbWord> result)
{
    QString buf = "";

    if( result.length()==0 )
    {
        return buf;
    }

    for( int i=0 ; i<result.length()-1 ; i++ )
    {
        for( int j=0 ; j<word_list.length() ; j++ )
        {
            if( result[i].word==word_list[j] )
            {
                buf += QString::number(j);
                buf += "_";

                break;
            }
        }
    }

    QString last_word = result.last().word;
    for( int j=0 ; j<word_list.length() ; j++ )
    {
        if( last_word==word_list[j] )
        {
            buf += QString::number(j);
        }
    }

    return buf;
}

void AbChapar::printWords(QVector<AbWord> words)
{
    QString msg;

    for( int i=0 ; i<words.size() ; i++ )
    {
        msg += words[i].word + "(";
        msg += QString::number(words[i].word_id) + ") ";
    }
    qDebug() << "Message:" << msg;
}

AbChapar::~AbChapar()
{
}
