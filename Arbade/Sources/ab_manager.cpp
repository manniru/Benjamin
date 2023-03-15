#include "ab_manager.h"
#include <time.h>
#include <stdlib.h>
#include <QQmlProperty>

AbManager::AbManager(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    srand(time(NULL));
    params.rec_time = QQmlProperty::read(root, "ab_rec_time").toFloat();
    int sample_count = params.rec_time*BT_REC_RATE;
    qDebug() << "sample_count" << sample_count;
    rec = new AbRecorder(sample_count);
    wav_wr = new AbWavWriter(rec->cy_buf, sample_count);
    wav_rd = new AbWavReader(rec->cy_buf, sample_count);
    connect(rec, SIGNAL(finished()), this, SLOT(writeWav()));
    connect(rec, SIGNAL(updatePercent(int)),
            this, SLOT(updateTime(int)));
    lexicon = bt_parseLexicon(BT_WORDLIST_PATH);
    loadWordList();
    read_timer = new QTimer();
    connect(read_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));
}

// verification and playing phase
void AbManager::readWave(QString filename)
{
    wav_rd->read(filename);
    QQmlProperty::write(root, "ab_rec_time", wav_rd->wave_time);
    QQmlProperty::write(root, "ab_power", wav_rd->power_dB);

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
    QQmlProperty::write(root, "ab_num_words", len);
    QQmlProperty::write(root, "ab_words", words);
}

// recording phase
void AbManager::writeWav()
{
    if( params.count<params.total_count )
    {
        if( params.status==AB_STATUS_REQPAUSE )
        {
            setStatus(AB_STATUS_PAUSE);
//            qDebug() << "writeWav:AB_STATUS_PAUSE";
        }
        else
        {
            setStatus(AB_STATUS_BREAK);
//            qDebug() << "writeWav:AB_STATUS_BREAK";
            QQmlProperty::write(root, "ab_elapsed_time", 0);
        }
    }
    else
    {
        setStatus(AB_STATUS_STOP);
//        qDebug() << "writeWav:AB_STATUS_STOP";
        params.count = 0;
    }

    double power_dB = calcPower(rec->cy_buf,
                                params.rec_time*BT_REC_RATE);
    QQmlProperty::write(root, "ab_power", power_dB);
    wav_wr->write(wav_path);

    if( params.count<params.total_count &&
        params.status==AB_STATUS_BREAK )
    {
        record();
    }
}

void AbManager::record()
{
    params.count++;
    QQmlProperty::write(root, "ab_count", params.count);
    wav_path = getRandPath(params.category);
    wav_wr->setCategory(params.category);
    setStatus(AB_STATUS_BREAK);
//    qDebug() << "record:AB_STATUS_BREAK";
    QQmlProperty::write(root, "ab_elapsed_time", 0);
    read_timer->start(params.pause_time*1000);
}

void AbManager::swapParams()
{
    if( params.verifier )
    {
        float pause_time = p_backup.pause_time;

        p_backup.pause_time = params.pause_time;
        p_backup.num_words = params.num_words;
        p_backup.rec_time = params.rec_time;
        p_backup.category = params.category;
        p_backup.total_count = params.total_count;

        QQmlProperty::write(root, "ab_pausetime", pause_time);
    }
    else
    {
        float pause_time = p_backup.pause_time;
        float num_words = p_backup.num_words;
        float rec_time = p_backup.rec_time;
        QString category = p_backup.category;
        float total_count = p_backup.total_count;

        p_backup.pause_time = params.pause_time;

        params.pause_time = pause_time;
        params.num_words = num_words;
        params.rec_time = rec_time;
        params.category = category;
        params.total_count = total_count;

        QQmlProperty::write(root, "ab_pausetime", pause_time);
        QQmlProperty::write(root, "ab_num_words", num_words);
        QQmlProperty::write(root, "ab_rec_time", rec_time);
        QQmlProperty::write(root, "ab_category", category);
        QQmlProperty::write(root, "ab_total_count", total_count);
    }
}

void AbManager::breakTimeout()
{
    if( params.status==AB_STATUS_REQPAUSE )
    {
        setStatus(AB_STATUS_PAUSE);
//        qDebug() << "readDone:AB_STATUS_PAUSE";
    }
    else
    {
        qDebug() << "start record";
        setStatus(AB_STATUS_REC);
//        qDebug() << "readDone:AB_STATUS_REC";
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

    if( params.focus_word>=0 && params.focus_word<lexicon.size() )
    {
        fix_word_index = rand()%AB_WORD_LEN;
    }

    while( true )
    {
        for( int i=0 ; i<AB_WORD_LEN ; i++ )
        {
            if( fix_word_index==i )
            {
                word_id[i] = params.focus_word;
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
    QQmlProperty::write(root, "ab_elapsed_time", percent);
}

void AbManager::copyToOnline(QString filename)
{
    QFile file(filename);
    QFileInfo unver_file(file);
    file.copy(KAL_AU_DIR"train/online/" +
              unver_file.fileName());
    file.remove();
}

QString AbManager::readWordList()
{
    QFile words_file(BT_WORDLIST_PATH);
    if( !words_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << BT_WORDLIST_PATH;
        return "";
    }
    QString ret = QString(words_file.readAll());
    words_file.close();
    return ret;
}

void AbManager::writeWordList()
{
    QFile words_file(BT_WORDLIST_PATH);
    if( !words_file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << BT_WORDLIST_PATH;
    }
    words_file.write(params.word_list.toStdString().c_str());
    words_file.close();
}

void AbManager::loadWordList()
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

void AbManager::delWordSamples()
{
    QStringList dif_words = params.dif_words.split("\n");
    int len = dif_words.length();
    QVector<int> del_list;
    for( int i=0 ; i<len ; i++ )
    {
        dif_words[i] = dif_words[i].split(".")[1].split("(")[0].trimmed();
        int result = wordToIndex(dif_words[i]);
        if( result>=0 && result<lexicon.size() )
        {
            del_list.push_back(result);
        }
    }
    len = del_list.size();
    if( len==0 )
    {
        return;
    }
    qDebug() << "delWordSamples" << del_list;

    QFileInfoList dir_list = ab_getAudioDirs();
    int len_dir = dir_list.size();
    if( len_dir==0 )
    {
        return;
    }
    for( int i=0 ; i<len_dir ; i++ )
    {
        QFileInfoList files_list = ab_listFiles(dir_list[i].
                                                absoluteFilePath());
        int len_files = files_list.size();
        for( int j=0 ; j<len_files ; j++ )
        {
            QStringList audio_words = files_list[j].baseName().split("_");
            for( int k=0 ; k<len ; k++ )
            {
                if( audio_words.contains(QString::number(del_list[k])) )
                {
                    QFile removing_file(files_list[j].absoluteFilePath());
                    qDebug() << "del" << files_list[j].absoluteFilePath();
                    removing_file.remove();
                }
            }
        }
    }
}

int AbManager::wordToIndex(QString word)
{
    int len = lexicon.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( lexicon[i]==word )
        {
            return i;
        }
    }
    return -1;
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

QString AbManager::idToWords(int id)
{
    return word_list[id];
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
    QQmlProperty::write(root, "ab_words", total_words.trimmed());
}

AbManager::~AbManager()
{
}

void AbManager::setStatus(int status)
{
    if( status==AB_STATUS_STOP )
    {
        QString stat = ab_getStat(params.category);
        QQmlProperty::write(root, "ab_word_stat", stat);
        QString meanvar = ab_getMeanVar();
        QQmlProperty::write(root, "ab_mean_var", meanvar);
    }
    params.status = status;
    QQmlProperty::write(root, "ab_status", status);
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
