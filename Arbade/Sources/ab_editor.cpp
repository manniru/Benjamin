#include "ab_editor.h"
#include <QQmlProperty>
#include <QDebug>

AbEditor::AbEditor(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    stat = new AbStat(root);
    editor = root->findChild<QObject *>("WordList");
    buttons = root->findChild<QObject *>("Buttons");

    connect(editor, SIGNAL(wordAdded(int)), this, SLOT(addWord(int)));
    connect(buttons, SIGNAL(saveClicked()), this, SLOT(saveProcess()));
    connect(buttons, SIGNAL(resetClicked()), this, SLOT(resetProcess()));
}

void AbEditor::addWord(int id)
{
    if( id<word_lines.length()+1 )
    {
        word_lines.resize(id+1);
    }

    QString object_name = "WordLine" + QString::number(id);
    word_lines[id] = editor->findChild<QObject *>(object_name);

    connect(word_lines[id], SIGNAL(wordChanged(int,QString)),
            this, SLOT(changeWord(int,QString)));
}

void AbEditor::changeWord(int id, QString text)
{
//    for last add line and last word line add box
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
    QString category = QQmlProperty::read(editor, "category").toString();
    qDebug() << "chera miay tu in";
//    stat->createWordEditor(category);
}

QString AbEditor::getDif()
{
    QString dif, word_new, word_old;
    QString ab_word_list = QQmlProperty::read(root, "ab_word_list").toString();
    QStringList words_old = ab_word_list.split("\n");
    int count = 0;
    int len_words_new = word_lines.size();
    int len_words_old = words_old.length();
    int max_len = (len_words_new>len_words_old)?len_words_new:len_words_old;

    for( int i=0 ; i<max_len ; i++ )
    {
        if( i>=len_words_new )
        {
            word_new = "<new>";
        }
        else
        {
            word_new = QQmlProperty::read(word_lines[i], "word_text").toString();
        }
        if( i>=len_words_old )
        {
            word_old = "<deleted>";
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
        }
    }
    return dif;
}

void AbEditor::statAll()
{
    QVector<int> word_count = stat->getAllCount();
    int len_stat = word_count.length();
    int len_lines = word_lines.length();
    for( int i=0 ; i<len_stat ; i++ )
    {
        if( i<len_lines )
        {
            QQmlProperty::write(word_lines[i],
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
    int len_lines = word_lines.length();
    for( int i=0 ; i<len_stat ; i++ )
    {
        if( i<len_lines )
        {
            QQmlProperty::write(word_lines[i],
                                "word_count", word_count[i]);
        }
    }
    stat->updateMeanVar(&word_count);
}
