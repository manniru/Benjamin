#include "ab_editor.h"
#include <QQmlProperty>
//#include <QGuiApplication>
#include <QDebug>
#include <QThread>

AbEditor::AbEditor(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    stat = new AbStat(root);
    editor = root->findChild<QObject *>("WordList");
    buttons = root->findChild<QObject *>("Buttons");
    rec_list = root->findChild<QObject *>("RecList");

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));

    connect(editor, SIGNAL(wordAdded(int)), this, SLOT(wordAdded(int)));
    connect(rec_list, SIGNAL(wordAdded(int)), this, SLOT(wordAddedRec(int)));
    connect(editor, SIGNAL(updateWordList()),
            this, SLOT(writeWordList()));
    connect(buttons, SIGNAL(saveClicked()), this, SLOT(saveProcess()));
    connect(buttons, SIGNAL(resetClicked()), this, SLOT(resetProcess()));
}

void AbEditor::wordAdded(int id)
{
    if( id>=editor_lines.length() )
    {
        editor_lines.resize(id+1);
    }

    QString object_name = "WordLine" + QString::number(id);
    editor_lines[id] = editor->findChild<QObject *>(object_name);

    connect(editor_lines[id], SIGNAL(wordChanged(int,QString)),
            this, SLOT(changeWord(int,QString)));
}

void AbEditor::wordAddedRec(int id)
{
    if( id>=rec_lines.length() )
    {
        rec_lines.resize(id+1);
    }

    QString object_name = "RecLine" + QString::number(id);
    rec_lines[id] = rec_list->findChild<QObject *>(object_name);

    connect(rec_lines[id], SIGNAL(removeClicked(int)),
            this, SLOT(recRemove(int)));
}

void AbEditor::recRemove(int id)
{
    QString path = QQmlProperty::read(rec_lines[id], "path").toString();
    qDebug() << "ID = , path = " << id << path;

    QVariant id_v(id);
    QGenericArgument arg_id  = Q_ARG(QVariant, id_v);
    QMetaObject::invokeMethod(rec_list, "removeLine", arg_id);

    rec_lines.remove(id);
//    QFile removing_file(path);
//    if( removing_file.exists() )
//    {
//        removing_file.remove();
//    }
}

void AbEditor::changeWord(int id, QString text)
{
//    for last add line and last word line add box
    if( id==editor_lines.length()-1 && text.length() )
    {
        stat->addWord("", -1, AB_COLOR_NORM);
    }
    else if( id==editor_lines.length()-2 && text.length()==0 )
    {
        QMetaObject::invokeMethod(editor, "removeWord");
        editor_lines.removeLast();
    }
    enableButtons();
}

void AbEditor::enableButtons()
{
    QQmlProperty::write(buttons, "btn_enable", true);
}

void AbEditor::saveProcess()
{
    QString dif = getDif();
    if( dif.length() )
    {
        QVariant dif_v(dif);
        QGenericArgument arg_dif  = Q_ARG(QVariant, dif_v);
        QMetaObject::invokeMethod(editor, "launchDialog", arg_dif);
    }
}

void AbEditor::resetProcess()
{
    QMetaObject::invokeMethod(editor, "clearEditor");
    editor_lines.clear();
    // Qt QML destroy bug forced us to exert a timer
    // to make sure that the "clearEditor" function
    // made its clearance
    timer->start(10);
}

void AbEditor::timerTimeout()
{
    QString category = QQmlProperty::read(editor, "category").toString();
    stat->createWordEditor(category);
    timer->stop();
}

QString AbEditor::getDif()
{
    QString dif, word_new, word_old;
    QStringList words_old = stat->lexicon;
    int max_len, count = 0;
    int len_words_new = editor_lines.size()-1;
    int len_words_old = words_old.length();

    if( len_words_new>len_words_old )
    {
        max_len = len_words_new;
    }
    else
    {
        max_len = len_words_old;
    }

    for( int i=0 ; i<max_len ; i++ )
    {
        if( i>=len_words_new )
        {
            word_new = "<deleted>";
        }
        else
        {
            word_new = QQmlProperty::read(editor_lines[i], "word_text").toString();
        }
        if( i>=len_words_old )
        {
            word_old = "<new>";
        }
        else
        {
            word_old = words_old[i];
        }

        if( word_new!=word_old )
        {
            count++;
            dif += QString::number(count) + ". " + word_old;
            dif += "(" + QString::number(i) + ")";
            dif += " => " + word_new;
            dif += "\n";
        }
    }
    return dif.trimmed();
}

QString AbEditor::getUiWordList()
{
    QString ret;
    int len = editor_lines.size();
    ret.reserve(len);

    for( int i=0 ; i<len ; i++ )
    {
        ret += QQmlProperty::read(editor_lines[i], "word_text").toString();
        ret += "\n";
    }
    return ret.trimmed();
}

void AbEditor::writeWordList()
{
    QString wl_path = ab_getAudioPath() + "..\\word_list";
    QFile words_file(wl_path);
    if( !words_file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << wl_path;
    }
    QString word_list = getUiWordList();
    words_file.write(word_list.toStdString().c_str());
    words_file.close();
    stat->parseLexicon();
}

void AbEditor::statAll()
{
    QVector<int> word_count = stat->getAllCount();
    int len_stat = word_count.length();
    int len_lines = editor_lines.length();
    for( int i=0 ; i<len_stat ; i++ )
    {
        if( i<len_lines )
        {
            QQmlProperty::write(editor_lines[i],
                                "word_count", word_count[i]);
        }
    }
    stat->updateMeanVar(&word_count);
}

void AbEditor::updateStat()
{
    QString category = QQmlProperty::read(editor, "category").toString();
    QVector<int> word_count = stat->getCategoryCount(category);
    int len_stat = word_count.length();
    int len_lines = editor_lines.length();
    for( int i=0 ; i<len_stat ; i++ )
    {
        if( i<len_lines )
        {
            QQmlProperty::write(editor_lines[i],
                                "word_count", word_count[i]);
        }
    }
    stat->updateMeanVar(&word_count);
}
