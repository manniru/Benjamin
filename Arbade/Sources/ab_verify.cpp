#include "ab_verify.h"
#include <QQmlProperty>
#include <QGuiApplication>

AbVerify::AbVerify(AbEditor *ed, QObject *ui, QObject *parent): QObject(parent)
{
    root = ui;
    editor = ed;
    wav_rd = new AbWavReader;

    query = root->findChild<QObject *>("Query");

    connect(query, SIGNAL(generate()),
            this, SLOT(generateWrongForms()));
    connect(query, SIGNAL(keyPress(int)),
            this, SLOT(execWrongKey(int)));

    connect(root, SIGNAL(delVerifyFile()), this, SLOT(deleteFile()));
    connect(root, SIGNAL(copyUnverifyFile()), this, SLOT(moveToOnline()));
    connect(root, SIGNAL(trashVerifyFile()), this, SLOT(trashFile()));

    QString wrong_path = ab_getAudioPath() + "wrong";
    QDir wrong_dir(wrong_path);

    if( !wrong_dir.exists() )
    {
        qDebug() << "Creating" << wrong_path
                 << " Directory";
#ifdef WIN32
        QString cmd = "mkdir " + wrong_path;
        system(cmd.toStdString().c_str());
#else //OR __linux
        system("mkdir -p " KAL_AU_DIR "wrong");
#endif
    }
}

void AbVerify::generateWrongForms()
{
    w_shortcut.clear();
    w_word.clear();
    w_path.clear();

    int id = getId();
    QString file_path = editor->stat->cache_files[0][id];
    QFileInfo info(file_path);
    QString filename = info.fileName();
    filename = filename.remove(".wav");
    QStringList name_extended = filename.split(".");
    filename = info.baseName();

    w_path = createWrongList(filename);
    w_path.removeLast();
    std::sort(w_path.begin(), w_path.end());

    QString wrong_path = ab_getAudioPath() + "wrong\\";
    int len = w_path.length();
    for( int i=0 ; i<len; i++ )
    {
        if( name_extended.size()>1 )
        {
            w_word << idToWord(w_path[i], name_extended[1]);
            w_path[i] = wrong_path + w_path[i];
            w_path[i] += "." + name_extended[1] + ".wav";
        }
        else
        {
            w_word << idToWord(w_path[i], "");
            w_path[i] = wrong_path + w_path[i];
            w_path[i] += ".wav";
        }

        if( i<10 )
        {
            w_shortcut << QString::number(i);
        }
        else
        {
            w_shortcut << QString('a' + (i-10));
        }

        addWrongForm(w_word[i], w_path[i], w_shortcut[i]);
    }

    QMetaObject::invokeMethod(query, "addCompleted");
}

void AbVerify::addWrongForm(QString w_word, QString w_path, QString shortcut)
{
    QVariant word_v(w_word);
    QVariant path_v(w_path);
    QVariant shortcut_v(shortcut);

    QGenericArgument arg_word     = Q_ARG(QVariant, word_v);
    QGenericArgument arg_path     = Q_ARG(QVariant, path_v);
    QGenericArgument arg_shortcut = Q_ARG(QVariant, shortcut_v);

    QMetaObject::invokeMethod(query, "addForm", arg_word, arg_path, arg_shortcut);
}

QVector<QString> AbVerify::createWrongList(QString in)
{
    QStringList words = in.split("_");
    int len = words.length();
    QVector<QString> wrong_list;

    if( len==1 )
    {
        wrong_list << "-" + in;
        wrong_list << in;
        return wrong_list;
    }
    else
    {
        QString rest;
        for( int i=1 ; i<len ; i++ )
        {
            rest += words[i];
            if( i<len-1 )
            {
                rest += "_";
            }
        }

        QVector<QString> rest_wrong = createWrongList(rest);
        int rest_len = rest_wrong.length();
        for( int i=0 ; i< rest_len ; i++ )
        {
            QString wrong_rest = "-" + words[0] + "_" + rest_wrong[i];
            QString right_rest = words[0] + "_" + rest_wrong[i];
            wrong_list << wrong_rest;
            wrong_list << right_rest;
        }

        return wrong_list;
    }
}

QString AbVerify::idToWord(QString filename, QString id)
{
    QStringList words = filename.split('_');
    int words_len = words.length();
    QString ret;
    for( int j=0 ; j<words_len ; j++)
    {
        if( words[j].front()==QChar('-') )
        {
            ret += "<wrong> ";
        }
        else
        {
            int num = words[j].toInt();
            ret += "<";
            ret += editor->stat->lexicon[num] + "> ";
        }
    }
    ret = ret.trimmed();

    if( id.length() )
    {
        ret += "(" + id + ")";
    }

    qDebug() << "word = " << ret;
    return ret;
}

void AbVerify::execWrongKey(int key)
{
    int id = -1;

    if( Qt::Key_0<=key && key<=Qt::Key_9 )
    {
        id = key - Qt::Key_0;
    }
    else if( Qt::Key_A<=key && key<Qt::Key_Z )
    {
        id = key - Qt::Key_A + 10;
    }
    else if( key==Qt::Key_Z )
    {
        moveToOnline();
        recRemove();
        return;
    }

    if( id>=w_path.size() )
    {
        return;
    }

    int cache_id = getId();
    QString old_path = editor->stat->cache_files[0][cache_id];
    QString new_path = w_path[id];
    QFile file(old_path);
    file.copy(new_path);
//    file.remove();
    editor->stat->deleteCache(AB_UNVER_ID, cache_id);
    recRemove();
}

QString AbVerify::wrongAll(QString file_path)
{
    QFileInfo info(file_path);
    QString filename = info.fileName();
    filename = filename.remove(".wav");
    QStringList name_extended = filename.split(".");
    filename = name_extended[0];

    QStringList words = filename.split("_");
    int len = words.size();
    for( int i=0 ; i<len ; i++ )
    {
        words[i] = "-" + words[i];
    }
    filename = words.join("_");
    if( name_extended.size()>1 )
    {
        filename += "." + name_extended[1];
    }
    filename += ".wav";
    QString dir_path = ab_getAudioPath() + "wrong\\";
    return dir_path + filename;
}

void AbVerify::moveToOnline()
{
    checkOnlineExist();

    int id = getId();
    QString file_path = editor->stat->cache_files[0][id];
    QString online_path = ab_getAudioPath() + "train\\online\\";
    QFile file(file_path);
    online_path += file.fileName();
    file.copy(online_path);
//    file.remove();
    editor->stat->moveToOnline(id);

    recRemove();
}

void AbVerify::deleteFile()
{
    int id = getId();
    QString file_path = editor->stat->cache_files[0][id];
    QFile file(file_path);
    QString new_path = wrongAll(file_path);
    file.copy(new_path);
//    file.remove();
    editor->stat->deleteCache(AB_UNVER_ID, id);
    recRemove();
}

void AbVerify::trashFile()
{
    int id = getId();
    QString file_path = editor->stat->cache_files[0][id];
    QFile file(file_path);
//    file.remove();
    editor->stat->deleteCache(AB_UNVER_ID, id);
    recRemove();
}

void AbVerify::checkOnlineExist()
{
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
}

void AbVerify::recRemove()
{
    int count = QQmlProperty::read(root, "ab_count").toInt();
    int total_count = QQmlProperty::read(root, "ab_total_count_v").toInt();
    int rec_id  = total_count-count;
    editor->recRemove(rec_id, 0);
}

// verification and playing phase
void AbVerify::loadNext()
{
    QString filename = getNext();
    double power = wav_rd->getPower(filename);
    QQmlProperty::write(root, "ab_power", power);
    QQmlProperty::write(root, "ab_address", filename);

    QFileInfo wav_file(filename);
    QString basename = wav_file.baseName();
    QStringList id_strlist = basename.split("_", QString::SkipEmptyParts);
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

QString AbVerify::getNext()
{
    int focus_word = QQmlProperty::read(root, "ab_focus_word_v").toInt();
    QString filename;

    if( focus_word==-1 ) //no focus word
    {
        filename = editor->stat->cache_files[0].last();
    }
    else
    {
        int curr_id = QQmlProperty::read(root, "ab_verify_id").toInt();

        if( curr_id==0 )
        {
            int len = editor->stat->cache_files[0].length();
            curr_id = len-1;
            qDebug() << "curr_id" << filename << curr_id;
        }

        while( curr_id>0 )
        {
            curr_id--;
            filename = editor->stat->cache_files[0][curr_id];
            QFileInfo info(filename);
            QStringList split = info.baseName().split("_");
            int split_len = split.length();
            for( int i=0 ; i<split_len ; i++ )
            {
                if( split[i].toInt()==focus_word )
                {
                    QQmlProperty::write(root, "ab_verify_id", curr_id);
                    return filename;
                }
            }
            filename = "";
        }
        //handle marginal cond
    }

    return filename;
}

int AbVerify::getId()
{
    int focus_word = QQmlProperty::read(root, "ab_focus_word_v").toInt();
    int ret;

    if( focus_word==-1 ) //no focus word
    {
        ret = editor->stat->cache_files[0].length()-1;
    }
    else
    {
        ret = QQmlProperty::read(root, "ab_verify_id").toInt();
    }

    return ret;
}

QString AbVerify::idsToWords(QVector<int> ids)
{
    int len_id = ids.size();
    QString ret;
    for( int i=0 ; i<len_id ; i++ )
    {
        ret += "<" + editor->stat->lexicon[ids[i]] + "> ";
    }
    return ret.trimmed();
}
