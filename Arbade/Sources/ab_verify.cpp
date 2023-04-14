#include "ab_wrong.h"
#include <QQmlProperty>
#include <QGuiApplication>

AbWrong::AbWrong(AbStat *st, QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    stat = st;

    query = root->findChild<QObject *>("Query");

    connect(query, SIGNAL(generate()),
            this, SLOT(generateWrongForms()));
    connect(query, SIGNAL(keyPress(int)),
            this, SLOT(processKey(int)));
}

void AbWrong::generateWrongForms()
{
    w_shortcut.clear();
    w_word.clear();
    w_path.clear();

    QString file_path = stat->cache_files[0].last();
    QFileInfo info(file_path);
    QString filename = info.fileName();
    filename = filename.remove(".wav");
    QStringList name_extended = filename.split(".");
    filename = info.baseName();

    w_path = createList(filename);
    w_path.removeLast();
    std::sort(w_path.begin(), w_path.end());

    int len = w_path.length();
    for( int i=0 ; i<len; i++ )
    {
        if( name_extended.size()>1 )
        {
            w_word << idToWord(w_path[i], name_extended[1]);
        }
        else
        {
            w_word << idToWord(w_path[i], "");
        }

        if( i<10 )
        {
            w_shortcut << QString::number(i);
        }
        else
        {
            w_shortcut << QString('a' + (i-10));
        }

        addForm(w_word[i], w_path[i], w_shortcut[i]);
    }

    QMetaObject::invokeMethod(query, "addCompleted");
}

void AbWrong::addForm(QString w_word, QString w_path, QString shortcut)
{
    QVariant word_v(w_word);
    QVariant path_v(w_path);
    QVariant shortcut_v(shortcut);

    QGenericArgument arg_word     = Q_ARG(QVariant, word_v);
    QGenericArgument arg_path     = Q_ARG(QVariant, path_v);
    QGenericArgument arg_shortcut = Q_ARG(QVariant, shortcut_v);

    QMetaObject::invokeMethod(query, "addForm", arg_word, arg_path, arg_shortcut);
}

QVector<QString> AbWrong::createList(QString in)
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

        QVector<QString> rest_wrong = createList(rest);
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

QString AbWrong::idToWord(QString filename, QString id)
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
            ret += stat->lexicon[num] + "> ";
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

void AbWrong::processKey(int key)
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
        qDebug() << "Key_Z";
    }

    qDebug() << "event.key =" << id;
}
