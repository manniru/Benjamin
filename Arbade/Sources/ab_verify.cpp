#include "ab_verify.h"
#include <QQmlProperty>
#include <QGuiApplication>
#include <sys/types.h>
#include <utime.h>

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
    connect(root, SIGNAL(verifierChanged()), this, SLOT(updateVerifier()));

    ab_checkAuDir("wrong");
}

void AbVerify::generateWrongForms()
{
    w_shortcut.clear();
    w_word.clear();
    w_path.clear();

    int id = getId();
    QString file_path = editor->cache->cache_files[verify_id][id];
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
    if( len>15 )
    {
        len = 15;
    }

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
            w_shortcut << QString(QChar('a' + (i-10)));
            qDebug() << "w_shortcut" << w_shortcut[i];
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

//    qDebug() << "word = " << ret;
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
        trashFile();
        recRemove();
        return;
    }
    else
    {
        return;
    }

    if( id>=w_path.size() )
    {
        return;
    }

    int cache_id = getId();
    QString old_path = editor->cache->cache_files[verify_id][cache_id];
    QString new_path = w_path[id];
    QFile file(old_path);
    file.copy(new_path);
    file.remove();
    editor->cache->deleteCache(AB_UNVER_ID, cache_id);
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
    QString file_path = editor->cache->cache_files[verify_id][id];
    QString online_path = ab_getAudioPath() + "train\\online\\";
    QFile file(file_path);
    QFileInfo info(file_path);
    online_path += info.fileName();
    file.copy(online_path);
    // change last modification time to now
    utime(online_path.toStdString().c_str(), NULL);

    file.remove();
    editor->stat->moveToOnline(id);
    recRemove();
}

void AbVerify::deleteFile()
{
    int id = getId();
    QString file_path = editor->cache->cache_files[verify_id][id];
    QFile file(file_path);
    QString new_path = wrongAll(file_path);
    file.copy(new_path);
//    file.remove();
    editor->cache->deleteCache(verify_id, id);
    recRemove();
}

void AbVerify::trashFile()
{
    int id = getId();
    QString file_path = editor->cache->cache_files[verify_id][id];
    QFile file(file_path);
    file.remove();
    editor->cache->deleteCache(verify_id, id);
    recRemove();
}

void AbVerify::checkOnlineExist()
{
    QString dirname = "train";
    dirname += QDir::separator();
    dirname += "online";
    ab_checkAuDir(dirname);
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
    int focus_word = QQmlProperty::read(root, "ab_focus_word_v").toInt();
    QString filename;
    if( focus_word==-1 )
    {
        filename = editor->cache->cache_files[verify_id].last();
    }
    else
    {
        filename = getFNext(focus_word);
    }
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

// get next sample for focused word
QString AbVerify::getFNext(int focus_word)
{
    QString filename;
    int curr_id = QQmlProperty::read(root, "ab_verify_id").toInt();

    if( curr_id==0 )
    {
        int len = editor->cache->cache_files[verify_id].length();
        curr_id = len;
        qDebug() << "curr_id" << filename << curr_id;
    }

    while( curr_id>0 )
    {
        curr_id--;
        filename = editor->cache->cache_files[verify_id][curr_id];
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

    return filename;
}

int AbVerify::getId()
{
    int focus_word = QQmlProperty::read(root, "ab_focus_word_v").toInt();
    int ret;

    if( focus_word==-1 ) //no focus word
    {
        ret = editor->cache->cache_files[verify_id].length()-1;
    }
    else
    {
        ret = QQmlProperty::read(root, "ab_verify_id").toInt();
    }

    return ret;
}

void AbVerify::updateVerifier()
{
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    if( verifier==2 )
    {
        verify_id = 1;
    }
    else
    {
        verify_id = 0;
    }
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
