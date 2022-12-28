#include "ab_manager.h"
#include <time.h>
#include <stdlib.h>

AbManager::AbManager(QObject *parent) : QObject(parent)
{
    srand(time(NULL));
    int sample_count = params.rec_time*BT_REC_RATE;
    qDebug() << "sample_count" << sample_count;
    rec = new AbRecorder(sample_count);
    wav_wr = new AbWavWriter(rec->cy_buf, sample_count);
    wav_rd = new AbWavReader(rec->cy_buf, sample_count);
    connect(rec, SIGNAL(finished()), this, SLOT(writeWav()));
    connect(rec, SIGNAL(updatePercent(int)),
            this, SLOT(updateTime(int)));
    lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
    readWordList();
    read_timer = new QTimer();
    connect(read_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));
}

// verification and playing phase
void AbManager::readWave(QString filename)
{
    wav_rd->read(filename);
    emit recTimeChanged(wav_rd->wave_time);
    emit powerChanged(wav_rd->power_dB);

    QFileInfo wav_file(filename);
    filename = wav_file.baseName();
    QStringList id_strlist = filename.split("_", QString::SkipEmptyParts);
    int len = id_strlist.size();
    QVector<int> id_list;
    for( int i=0 ; i<len ; i++ )
    {
        id_list.push_back(id_strlist[i].toInt());
    }
    QString words = idsToWords(id_list);
    emit numWordChanged(len);
    emit wordsChanged(words);
}

// recording phase
void AbManager::writeWav()
{
    if( params.count<params.total_count )
    {
        if( params.status==AB_STATUS_REQPAUSE )
        {
            emit statusChanged(AB_STATUS_PAUSE);
//            qDebug() << "writeWav:AB_STATUS_PAUSE";
        }
        else
        {
            emit statusChanged(AB_STATUS_BREAK);
//            qDebug() << "writeWav:AB_STATUS_BREAK";
            emit timeChanged(0);
        }
    }
    else
    {
        emit statusChanged(AB_STATUS_STOP);
//        qDebug() << "writeWav:AB_STATUS_STOP";
        params.count = 0;
    }

    double power_dB = calcPower(rec->cy_buf,
                                params.rec_time*BT_REC_RATE);
    emit powerChanged(power_dB);
    wav_wr->write(wav_path);

    if( params.count<params.total_count &&
        params.status==AB_STATUS_BREAK )
    {
        record();
    }
}

void AbManager::record()
{
    emit countChanged(params.count + 1);
    wav_path = getRandPath(params.category);
    wav_wr->setCategory(params.category);
    emit statusChanged(AB_STATUS_BREAK);
//    qDebug() << "record:AB_STATUS_BREAK";
    emit timeChanged(0);
    read_timer->start(params.pause_time*1000);
}

void AbManager::swapParams()
{
    if( params.verifier )
    {
        qreal pause_time = p_backup.pause_time;

        p_backup.pause_time = params.pause_time;
        p_backup.num_words = params.num_words;
        p_backup.rec_time = params.rec_time;
        p_backup.category = params.category;
        p_backup.total_count = params.total_count;

        emit pauseChanged(pause_time);
    }
    else
    {
        qreal pause_time = p_backup.pause_time;
        qreal num_words = p_backup.num_words;
        qreal rec_time = p_backup.rec_time;
        QString category = p_backup.category;
        qreal total_count = p_backup.total_count;

        p_backup.pause_time = params.pause_time;

        emit pauseChanged(pause_time);
        emit numWordChanged(num_words);
        emit recTimeChanged(rec_time);
        emit categoryChanged(category);
        emit totalCountChanged(total_count);
    }
}

void AbManager::breakTimeout()
{
    if( params.status==AB_STATUS_REQPAUSE )
    {
        emit statusChanged(AB_STATUS_PAUSE);
//        qDebug() << "readDone:AB_STATUS_PAUSE";
    }
    else
    {
        emit statusChanged(AB_STATUS_REC);
//        qDebug() << "readDone:AB_STATUS_REC";
        qDebug() << "start record";
        rec->reset();
    }
    read_timer->stop();
}

QString AbManager::getRandPath(QString category)
{
    int word_id[AB_WORD_LEN];
    int lexicon_size = lexicon.length();
    QVector<AbWord> words;
    int fix_word_index = -1;

    if( params.focusword.length() )
    {
        fix_word_index = rand()%AB_WORD_LEN;
    }

    while( true )
    {
        for( int i=0 ; i<AB_WORD_LEN ; i++ )
        {
            if( fix_word_index==i )
            {
                bool ok=false;
                word_id[i] = params.focusword.toInt(&ok);
                if(!ok || word_id[i]<0 || word_id[i]>=lexicon_size )
                {
                    word_id[i] = rand()%lexicon_size;
                }
            }
            else
            {
                word_id[i] = rand()%lexicon_size;
            }
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

QString AbManager::idsToWords(QVector<int> ids)
{
    int len_id = ids.size();
    QString ret;
    for( int i=0 ; i<len_id ; i++ )
    {
        ret += "<" + word_list[ids[i]] + "> ";
    }
    return ret.trimmed();
}

void AbManager::printWords(QVector<AbWord> words)
{
    QString msg, total_words;

    for( int i=0 ; i<words.size() ; i++ )
    {
        msg += words[i].word + "(";
        msg += QString::number(words[i].word_id) + ") ";
        total_words += "<" + words[i].word + "> ";
        if( i%3==2 )
        {
            total_words += "\n";
        }
    }
    qDebug() << "Message:" << msg;
    emit wordsChanged(total_words.trimmed());
}

AbManager::~AbManager()
{
}

double calcPower(int16_t *buffer, int len)
{
    double sum_sq = 0;
    for( int i=0 ; i<len ; i++ )
    {
        sum_sq += pow(buffer[i],2);
    }
    double power = sqrt(sum_sq)/len;
    double power_dB = 20*log10(power);
    power_dB += 50; // calibration
    return power_dB;
}
