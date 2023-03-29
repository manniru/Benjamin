#include "ab_manager.h"
#include <time.h>
#include <stdlib.h>
#include <QQmlProperty>

AbManager::AbManager(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    editor = root->findChild<QObject *>("WordList");
    srand(time(NULL));

    audio = new AbAudio(root);
    stat  = new AbStat(root);

    connect(audio, SIGNAL(setStatus(int)), this, SLOT(setStatus(int)));
    connect(root, SIGNAL(deleteSample(QString)),
            this, SLOT(deleteSample(QString)));
}

void AbManager::readWave(QString filename)
{
    audio->readWave(filename);
}

void AbManager::record()
{
    audio->record();
}

void AbManager::copyToOnline(QString filename)
{
    QFile file(filename);
    QFileInfo unver_file(file);

    QString online_dir = ab_getAudioPath();
    online_dir += "train\\online";
    QDir au_TrainDir(online_dir);

    if( !au_TrainDir.exists() )
    {
        qDebug() << "Creating" << online_dir
                 << " Directory";
#ifdef WIN32
        QString cmd = "mkdir " + online_dir;
        system(cmd.toStdString().c_str());
#else //OR __linux
        system("mkdir -p " KAL_AU_DIR "train/online");
#endif
    }
    QString new_path = online_dir + unver_file.fileName();
    file.copy(new_path);
    file.remove();
}

QString AbManager::readWordList()
{
    QString wl_path = ab_getAudioPath() + "..\\word_list";
    QFile words_file(wl_path);
    if( !words_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << wl_path;
        return "";
    }
    QString ret = QString(words_file.readAll());
    words_file.close();
    return ret;
}

void AbManager::writeWordList()
{
    QString wl_path = ab_getAudioPath() + "..\\word_list";
    QFile words_file(wl_path);
    if( !words_file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << wl_path;
    }
    QString word_list = QQmlProperty::read(root, "ab_word_list").toString();
    words_file.write(word_list.toStdString().c_str());
    words_file.close();
    audio->parseLexicon();
}

void AbManager::delWordSamples()
{
    QString dif = QQmlProperty::read(root, "ab_dif_words").toString();
    QStringList dif_words = dif.split("\n");
    int len = dif_words.length();
    QVector<int> del_list;

    for( int i=0 ; i<len ; i++ )
    {
        dif_words[i] = dif_words[i].split(".")[1].split("(")[0].trimmed();
        int result = wordToIndex(dif_words[i]);

        if( result>=0 && result<audio->lexicon.size() )
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

    QFileInfoList dir_list = stat->getAudioDirs();
    int len_dir = dir_list.size();

    if( len_dir==0 )
    {
        return;
    }

    for( int i=0 ; i<len_dir ; i++ )
    {
        QFileInfoList files_list = stat->listFiles(dir_list[i].
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

void AbManager::deleteSample(QString sample)
{
    QString category = QQmlProperty::read(editor, "category")
                                        .toString();
    QString file_path = ab_getAudioPath() + "train\\";
    file_path += category + "\\";

    QStringList sample_words = sample.split(" ",
                                     QString::SkipEmptyParts);
    int len = sample_words.length();
    for( int i=0 ; i<len ; i++ )
    {
        sample_words[i] = sample_words[i].remove(">").remove("<");
        file_path += QString::number(wordToIndex(sample_words[i]));
        file_path += "_";
    }
    file_path.chop(1); // removes last '_'
    file_path += ".wav";
    deleteFile(file_path);
}

void AbManager::deleteFile(QString path)
{
    QFile removing_file(path);
    qDebug() << "del" << path;
    if( removing_file.exists() )
    {
        removing_file.remove();
    }
}

int AbManager::wordToIndex(QString word)
{
    int len = audio->lexicon.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( audio->lexicon[i]==word )
        {
            return i;
        }
    }
    return -1;
}

QString AbManager::idToWord(int id)
{
    if( id<audio->lexicon.count() )
    {
        return audio->lexicon[id];
    }
    else
    {
        return "<Unknown>";
    }
}

AbManager::~AbManager()
{
}

void AbManager::setStatus(int status)
{
    if( status==AB_STATUS_STOP )
    {
        QString category = QQmlProperty::read(editor, "category").toString();
        QString statis = stat->getStat(category);
        QQmlProperty::write(root, "ab_word_stat", statis);
    }

    QQmlProperty::write(root, "ab_status", status);
}
