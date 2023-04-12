#include "ab_audio.h"
#include <QQmlProperty>

AbAudio::AbAudio(AbStat *st, QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    stat = st;
    editor = root->findChild<QObject *>("WordList");
    float rec_time = QQmlProperty::read(root, "ab_rec_time").toFloat();
    int sample_count = rec_time*BT_REC_RATE;
//    qDebug() << "sample_count" << sample_count;

    rec = new AbRecorder(sample_count);
    wav_wr = new AbWavWriter(rec->cy_buf, sample_count);
    wav_rd = new AbWavReader(rec->cy_buf, sample_count);
    read_timer = new QTimer();
    connect(rec, SIGNAL(updatePercent(int)),
            this, SLOT(updateTime(int)));
    connect(rec, SIGNAL(finished()), this, SLOT(writeWav()));
    connect(read_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));
}

void AbAudio::record()
{
    QMetaObject::invokeMethod(root, "incCount"); // ab_count++
    QString category = QQmlProperty::read(editor, "category").toString();
    wav_path = getRandPath(category);
    wav_wr->setCategory(category);
    setStatus(AB_STATUS_BREAK);
//    qDebug() << "record:AB_STATUS_BREAK";
    QQmlProperty::write(root, "ab_elapsed_time", 0);
    float pause_time = QQmlProperty::read(root, "ab_rec_pause").toFloat();
    read_timer->start(pause_time*1000);
}

void AbAudio::stop()
{
    QQmlProperty::write(root, "ab_elapsed_time", 0);
    read_timer->stop();

// verification and playing phase
void AbAudio::updateVerifyParam(QString filename)
{
    wav_rd->read(filename);
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
    qDebug() << "ab_words" << words;
    QQmlProperty::write(root, "ab_words", words);
}

void AbAudio::updateTime(int percent)
{
    QQmlProperty::write(root, "ab_elapsed_time", percent);
}

void AbAudio::writeWav()
{
    int status = QQmlProperty::read(root, "ab_status").toInt();
    if( status==AB_STATUS_STOP )
    {
        return;
    }
    int total_count = QQmlProperty::read(root, "ab_total_count").toInt();
    int count = QQmlProperty::read(root, "ab_count").toInt();

    if( count<total_count )
    {
        if( status==AB_STATUS_REQPAUSE )
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
        count = 0;
    }

    float rec_time = QQmlProperty::read(root, "ab_rec_time").toFloat();
    double power_dB = calcPower(rec->cy_buf,
                                rec_time*BT_REC_RATE);
    QQmlProperty::write(root, "ab_power", power_dB);

    checkCategoryExist();

    wav_wr->write(wav_path);
    QString wav_wpath = wav_path.replace("\\", "/");
    stat->addRecList(total_words, wav_wpath);

    status = QQmlProperty::read(root, "ab_status").toInt();
    if( count<total_count &&
        status==AB_STATUS_BREAK )
    {
        record();
    }
}

void AbAudio::checkCategoryExist()
{
    QString category = QQmlProperty::read(editor, "category").toString();
    QString base_name = ab_getAudioPath()+"train\\";
    base_name += category + "\\";
    QDir category_dir(base_name);

    if( !category_dir.exists() )
    {
        qDebug() << "Creating" << base_name
                 << " Directory";
        QString cmd;
#ifdef WIN32
        cmd = "mkdir " + base_name;
        system(cmd.toStdString().c_str());
#else //OR __linux
        cmd = "mkdir -p " KAL_AU_DIR "online";
        system(cmd.toStdString().c_str());
#endif
    }
}

void AbAudio::breakTimeout()
{
    int status = QQmlProperty::read(root, "ab_status").toInt();
    if( status==AB_STATUS_REQPAUSE )
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

QString AbAudio::idsToWords(QVector<int> ids)
{
    int len_id = ids.size();
    QString ret;
    for( int i=0 ; i<len_id ; i++ )
    {
        ret += "<" + stat->lexicon[ids[i]] + "> ";
    }
    return ret.trimmed();
}

QString AbAudio::getRandPath(QString category)
{
    int word_id[AB_WORD_LEN];
    int lexicon_size = stat->lexicon.length();
    QVector<AbWord> words;
    int fix_word_index = -1;
    int focus_word = QQmlProperty::read(root, "ab_focus_word").toInt();

    if( focus_word>=0 && focus_word<stat->lexicon.size() )
    {
        fix_word_index = rand()%AB_WORD_LEN;
    }

    while( true )
    {
        for( int i=0 ; i<AB_WORD_LEN ; i++ )
        {
            if( fix_word_index==i )
            {
                word_id[i] = focus_word;
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
            words[i].word = stat->lexicon[word_id[i]];
        }

        QString file_name = getFileName(words, category);

        if( QFile::exists(file_name)==0 )
        {
            showWords(words);
            return file_name;
        }
    }

    return "";
}

void AbAudio::showWords(QVector<AbWord> words)
{
    total_words = "";

    for( int i=0 ; i<words.size() ; i++ )
    {
        total_words += "<" + words[i].word + "> ";
    }
    total_words = total_words.trimmed();
    qDebug() << "total_words:" << total_words;
    QQmlProperty::write(root, "ab_words", total_words);
}

QString AbAudio::getFileName(QVector<AbWord> words,
                              QString category)
{
    // verified base name
    QString base_name = ab_getAudioPath() + "train\\";
    base_name += category + "\\";
    base_name += wordToId(words);
    QString name = base_name + ".wav";

    return name;
}

QString AbAudio::wordToId(QVector<AbWord> result)
{
    QString buf = "";

    if( result.length()==0 )
    {
        return buf;
    }

    for( int i=0 ; i<result.length()-1 ; i++ )
    {
        for( int j=0 ; j<stat->lexicon.length() ; j++ )
        {
            if( result[i].word==stat->lexicon[j] )
            {
                buf += QString::number(j);
                buf += "_";

                break;
            }
        }
    }

    QString last_word = result.last().word;
    for( int j=0 ; j<stat->lexicon.length() ; j++ )
    {
        if( last_word==stat->lexicon[j] )
        {
            buf += QString::number(j);
        }
    }

    return buf;
}

double AbAudio::calcPower(int16_t *buffer, int len)
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
