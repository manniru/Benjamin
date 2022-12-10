#include "ab_manager.h"
#include <QDebug>
#include <QDir>
#include <QThread>
#include <time.h>
#include <stdlib.h>

AbManager::AbManager(QObject *parent) : QObject(parent)
{
    srand(time(NULL));
    int sample_count = params.rec_time*BT_REC_RATE;
    qDebug() << "sample_count" << sample_count;
    rec = new AbRecorder(sample_count);
    wav = new AbWavWriter(rec->cy_buf, sample_count);
    connect(rec, SIGNAL(finished()), this, SLOT(writeWav()));
    connect(rec, SIGNAL(updatePercent(int)),
            this, SLOT(updateTime(int)));
    lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
    readWordList();
    read_timer = new QTimer();
    connect(read_timer, SIGNAL(timeout()),
            this, SLOT(readDone()));
}

void AbManager::writeWav()
{
    if( params.count<=params.total_count )
    {
        if( params.status==AB_STATE_REQPAUSE )
        {
            emit statusChanged(AB_STATE_PAUSE);
        }
        else
        {
            emit statusChanged(AB_STATE_BREAK);
            emit timeChanged(0);
        }
    }
    else
    {
        emit statusChanged(AB_STATE_STOP);
        params.count = 0;
    }
    wav->write(wav_path);

    if( params.count<=params.total_count &&
        params.status!=AB_STATE_REQPAUSE )
    {
        record();
    }
}

void AbManager::record()
{
    emit countChanged(params.count + 1);
    wav_path = getRandPath(params.category);
    wav->setCategory(params.category);
    emit statusChanged(AB_STATE_BREAK);
    emit timeChanged(0);
    read_timer->start(params.pause_time*1000);
}

void AbManager::readDone()
{
    if( params.status==AB_STATE_REQPAUSE )
    {
        emit statusChanged(AB_STATE_PAUSE);
    }
    else
    {
        emit statusChanged(AB_STATE_REC);
        qDebug() << "start record";
        rec->startStream();
    }
    read_timer->stop();
}

QString AbManager::getRandPath(QString category)
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

QString AbManager::getFileName(QVector<AbWord> words,
                              QString category)
{
    // verified base name
    QString base_name = KAL_AU_DIR"train/";
    base_name += category + "/";
    base_name += wordToId(words);
    QString name = base_name + ".wav";

    return name;
}

void AbManager::updateTime(int percent)
{
    emit timeChanged(percent);
}

void AbManager::readWordList()
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

QString AbManager::wordToId(QVector<AbWord> result)
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

void AbManager::printWords(QVector<AbWord> words)
{
    QString msg, total_words;

    for( int i=0 ; i<words.size() ; i++ )
    {
        msg += words[i].word + "(";
        msg += QString::number(words[i].word_id) + ") ";
        total_words += "<" + words[i].word + "> ";
    }
    qDebug() << "Message:" << msg;
    emit wordsChanged(total_words);
}

AbManager::~AbManager()
{
}
