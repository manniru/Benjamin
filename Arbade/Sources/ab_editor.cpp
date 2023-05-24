#include "ab_editor.h"
#include <QQmlProperty>
//#include <QGuiApplication>
#include <QDebug>
#include <QThread>

AbEditor::AbEditor(QObject *ui, AbTelegram *tel,
                   QObject *parent) : QObject(parent)
{
    root = ui;
    telegram = tel;
    cache = new AbCache();

    stat = new AbStat(root, cache, telegram);
    stat_thread = new QThread;
    stat->moveToThread(stat_thread);
    stat_thread->start();

    editor = root->findChild<QObject *>("WordList");
    buttons = root->findChild<QObject *>("Buttons");
    rec_list = root->findChild<QObject *>("RecList");
    message = root->findChild<QObject *>("Message");

    timer_editor = new QTimer();
    connect(timer_editor, SIGNAL(timeout()), this, SLOT(timerEdTimeout()));
    timer_rec = new QTimer();
    connect(timer_rec, SIGNAL(timeout()), this, SLOT(timerRecTimeout()));

    connect(editor, SIGNAL(wordAdded(int)), this, SLOT(wordAdded(int)));
    connect(rec_list, SIGNAL(wordAdded(int)), this, SLOT(wordAddedRec(int)));
    connect(editor, SIGNAL(updateWordList()),
            this, SLOT(writeWordList()));
    connect(buttons, SIGNAL(saveClicked()), this, SLOT(saveProcess()));
    connect(buttons, SIGNAL(resetClicked()), this, SLOT(resetProcess()));
    connect(this, SIGNAL(create(QString)), stat, SLOT(create(QString)));
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

void AbEditor::recRemove(int id, int f_focus)
{
    if( id>=rec_lines.length() )
    {
        //unbuffered index
        return;
    }
    QString path = QQmlProperty::read(rec_lines[id], "path").toString();

    QVariant id_v(id);
    QGenericArgument arg_id  = Q_ARG(QVariant, id_v);
    QVariant focus_v(f_focus);
    QGenericArgument arg_focus  = Q_ARG(QVariant, focus_v);
    QMetaObject::invokeMethod(rec_list, "removeLine", arg_id, arg_focus);

    rec_lines.remove(id);
    if( f_focus )
    {
        QFile removing_file(path);
        if( removing_file.exists() )
        {
            removing_file.remove();
        }
        QString category = getCategory();
        cache->deleteCache(category, path);
    }
    updateStat();
}

void AbEditor::changeWord(int id, QString text)
{
//    for last add line and last word line add box
    if( id==editor_lines.length()-1 && text.length() )
    {
        stat->addWord("", -1, "");
    }
    else if( id==editor_lines.length()-2 && text.length()==0 )
    {
        QMetaObject::invokeMethod(editor, "removeWord");
        editor_lines.removeLast();
    }
    else if( text.length() )
    {
        QString word = QQmlProperty::read(editor_lines[id], "word_text").toString();
        QString phoneme = stat->phoneme->getPhoneme(word);
        QQmlProperty::write(editor_lines[id], "word_phoneme", phoneme);
        if( phoneme=="" )
        {
            QQmlProperty::write(editor_lines[id], "wrong_phoneme", true);
        }
        else
        {
            QQmlProperty::write(editor_lines[id], "wrong_phoneme", false);
        }
    }
    enableButtons();
}

void AbEditor::enableButtons()
{
    QQmlProperty::write(buttons, "btn_enable", true);
}

void AbEditor::saveProcess()
{
//    int is_valid = 1; //is true if all words are in CMU Dict
    int len = editor_lines.length() - 1;
    for( int i=0 ; i<len ; i++ )
    {
//        QString word = QQmlProperty::read(editor_lines[i], "word_text").toString();
        QString phon = QQmlProperty::read(editor_lines[i], "word_phoneme").toString();
        if( phon.isEmpty() )
        {
            QString msg = "Some words phoneme are not available, ";
            msg += "The unavailable words are highlighted in red.\n";
            msg += "Please either change the words or add phonemes to CMU Dict.";
            QQmlProperty::write(message, "message", msg);
            return;
        }
    }

    dif_wordlist = getDif();
    if( dif_wordlist.length() )
    {
        QVariant dif_v(dif_wordlist);
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
    timer_editor->start(10);
}

void AbEditor::timerEdTimeout()
{
    QString category = getCategory();

    stat->createWordEditor(category);
    timer_editor->stop();
}

void AbEditor::timerRecTimeout()
{
    QString category = getCategory();

    stat->createRecList(category);
    timer_rec->stop();
}

void AbEditor::clearRecList()
{
    QMetaObject::invokeMethod(rec_list, "clearRecList");
    rec_lines.clear();
    // Qt QML destroy bug forced us to exert a timer
    // to make sure that the "clearEditor" function
    // made its clearance
    timer_rec->start(10);
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

QString AbEditor::getCategory()
{
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    QString category = AB_UNVER_DIR;
    if( verifier==2 )
    {
        category = AB_SHIT_DIR;
    }
    else if( verifier==0 )
    {
        category = QQmlProperty::read(editor, "category").toString();
    }
    return category;
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
    stat->lexicon = bt_parseLexicon();
}

void AbEditor::updateStatAll()
{
    int len_lines = editor_lines.length();
    QVector<int> word_count = stat->getAllCount();
    int len_stat = word_count.length();
    for( int i=0 ; i<len_stat ; i++ )
    {
        if( i<len_lines )
        {
            QQmlProperty::write(editor_lines[i],
                                "word_count", word_count[i]);
        }
    }
    stat->updateMeanVar(&word_count);
    int count = 0;
    int len_cat = AB_MAX_CAT;
    for( int i=1 ; i<len_cat ; i++ )
    {
        count += cache->cache_files[i].length();
    }
    QQmlProperty::write(editor, "count", count);
}

void AbEditor::updateStatCat()
{
    int len_lines = editor_lines.length();
    QString category = getCategory();
    QVector<int> *word_count = stat->getCategoryCount(category);
    int len_stat = word_count->length();
    for( int i=0 ; i<len_stat ; i++ )
    {
        if( i<len_lines )
        {
            QQmlProperty::write(editor_lines[i],
                                "word_count", word_count->at(i));
        }
    }
    stat->updateMeanVar(word_count);
    int id = cache->catToIndex(category);
    int count = cache->cache_files[id].length();
    QQmlProperty::write(editor, "count", count);
    qDebug() << "updateStatCat:" << count;
}

void AbEditor::updateStat()
{
    int len_lines = editor_lines.length();
    if( len_lines==0 )
    {
        //skip update before create on startup
        return;
    }

    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    int stat_all = QQmlProperty::read(root, "ab_all_stat").toInt();
    if( stat_all && verifier==0 )
    {
        updateStatAll();
    }
    else //all category stats
    {
        updateStatCat();
    }
}

void AbEditor::createList()
{
    QString category = getCategory();
    emit create(category);
}
