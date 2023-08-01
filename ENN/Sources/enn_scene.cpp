#include "enn_scene.h"
#include <QDebug>
#include <QDir>
#include <QQmlProperty>

EnnScene::EnnScene(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    sample_viewer = root->findChild<QObject *>("SamplesList");
    connect(sample_viewer, SIGNAL(sampleAdded(int)),
            this, SLOT(sampleAdded(int)));
    connect(root, SIGNAL(positionUpdated(int)),
            this, SLOT(updatePosition(int)));
    enn_path = ab_getAudioPath() + QDir::separator() + "enn";
    enn_path += QDir::separator();
    initSampleGrid();
    loadWordList();
}

void EnnScene::initSampleGrid()
{
    for( int i=0 ; i<SAMPLE_GRID_SIZE ; i++)
    {
        QVariant sample_v("");
        QGenericArgument arg_sample = Q_ARG(QVariant, sample_v);
        QMetaObject::invokeMethod(sample_viewer, "addSample",
                                  arg_sample);
    }
}

void EnnScene::loadWordList()
{
    word_list = bt_parseLexicon(BT_WORDS_PATH);
    wordlist_panel = root->findChild<QObject *>("wordList");
    connect(wordlist_panel, SIGNAL(loadEnnSamples(int)),
            this, SLOT(loadEnnSamples(int)));
    int len=word_list.size();
    for( int i=0 ; i<len ; i++ )
    {
        QVariant word_v(word_list[i]);
        QGenericArgument arg_word = Q_ARG(QVariant, word_v);
        QMetaObject::invokeMethod(wordlist_panel, "addLine",
                                  arg_word);
    }
}

void EnnScene::loadEnnSamples(int id)
{
    if( id>=0 && id<word_list.size() )
    {
        loadFiles(idToWord(id));
    }
}

void EnnScene::loadFiles(QString word)
{
    QString path = enn_path + word;
    files_list = ab_listFiles(path);

    int min_len;
    if( files_list.size()<SAMPLE_GRID_SIZE )
    {
        min_len = files_list.size();
    }
    else
    {
        min_len = SAMPLE_GRID_SIZE;
    }

    for( int i=0 ; i<min_len ; i++ )
    {
        QString sample_name = convertName(files_list[i]);
        QQmlProperty::write(sample_vector[i], "word_text",
                            sample_name);
        QQmlProperty::write(sample_vector[i], "word_id", i);
    }

    if( min_len<SAMPLE_GRID_SIZE )
    {
        for( int i=min_len ; i<SAMPLE_GRID_SIZE ; i++ )
        {
            QQmlProperty::write(sample_vector[i], "word_text", "");
            QQmlProperty::write(sample_vector[i], "word_id", -1);
        }
    }
    QQmlProperty::write(sample_viewer, "index", min_len);
    QQmlProperty::write(sample_viewer, "count", files_list.size());
}

void EnnScene::updatePosition(int direction)
{
    int first_id = QQmlProperty::read(sample_vector[0],
                                      "word_id").toInt();
    if( direction==Qt::Key_Down )
    {
        if( first_id+SAMPLE_GRID_SIZE>files_list.size() )
        {
            return;
        }
        int min_len;
        first_id += SAMPLE_GRID_SIZE;
        if( files_list.size()<first_id+SAMPLE_GRID_SIZE )
        {
            min_len = files_list.size();
        }
        else
        {
            min_len = first_id+SAMPLE_GRID_SIZE;
        }

        for( int i=first_id ; i<min_len ; i++ )
        {
            int sample_ui_id = i-first_id;
            QString sample_name = convertName(files_list[i]);
            QQmlProperty::write(sample_vector[sample_ui_id],
                                "word_text", sample_name);
            QQmlProperty::write(sample_vector[sample_ui_id],
                                "word_id", i);
        }

        if( min_len<first_id+SAMPLE_GRID_SIZE )
        {
            for( int i=min_len-first_id ; i<SAMPLE_GRID_SIZE ; i++ )
            {
                QQmlProperty::write(sample_vector[i], "word_text", "");
                QQmlProperty::write(sample_vector[i], "word_id", -1);
            }
        }
        QQmlProperty::write(sample_viewer, "index", min_len);
    }
    else
    {
        if( first_id==0 )
        {
            return;
        }
        first_id -= SAMPLE_GRID_SIZE;
        for( int i=first_id ; i<SAMPLE_GRID_SIZE ; i++ )
        {
            int sample_ui_id = i-first_id;
            QString sample_name = convertName(files_list[i]);
            QQmlProperty::write(sample_vector[sample_ui_id],
                                "word_text", sample_name);
            QQmlProperty::write(sample_vector[sample_ui_id],
                                "word_id", i);
        }
        QQmlProperty::write(sample_viewer, "index",
                            first_id+SAMPLE_GRID_SIZE);
    }
}

void EnnScene::sampleAdded(int id)
{
    if( id>=sample_vector.length() )
    {
        sample_vector.resize(id+1);
    }

    QString object_name = "Sample" + QString::number(id);
    sample_vector[id] = sample_viewer->
            findChild<QObject *>(object_name);
}

QString EnnScene::convertName(QString file_path)
{
    QFile file(file_path);
    QFileInfo file_info(file);
    QString file_name = file_info.fileName();

    QStringList name_split = file_name.split(".", Qt::SkipEmptyParts);
    QString first_part = name_split[0];
    int dot_split_size = name_split.size();
    if( dot_split_size<2 ) // name and extension
    {
        qDebug() << "Error 100: Bade file name" << file_name;
        return "";
    }

    QString num;
    int focus = -1;
    if( name_split[1]!="enn" )                // number and focus
    {
        QStringList num_split = name_split[1].split("_", Qt::SkipEmptyParts);
        if( num_split.size()<1 )
        {
            qDebug() << "Error 100-2: Bade file name" << file_name;
            return "";
        }

        if( num_split.size()>1 )
        {
            focus = num_split[1].toInt();
        }
        num = "." + num_split[0];
    }

    QStringList first_part_split = first_part.split("_", Qt::SkipEmptyParts);
    int len_split = first_part_split.size();
    if( len_split<2 ) // category + at least one word
    {
        qDebug() << "Error 100-1: Bade file name" << file_name;
        return "";
    }

    QString category = first_part_split[0];
    category.truncate(3); // first 3 characters
    QString ret = category + "/";
    for( int i=1 ; i<len_split ; i++ )       // words
    {
        int id = first_part_split[i].toInt();
        if( i-1==focus )
        {
            ret += "\"" + idToWord(id) + "\"_";
        }
        else
        {
            ret += idToWord(id) + "_";
        }
    }
    ret.chop(1); // remove extra "_"
    ret += num;
    return ret;
}

QString EnnScene::idToWord(int id)
{
    if( id<word_list.count() )
    {
        return word_list[id];
    }
    else
    {
        return "<Unknown>";
    }
}
