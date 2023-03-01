#include "ab_scene.h"
#include <QQmlProperty>

AbScene::AbScene(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    man = new AbManager(root);
    break_timer = new QTimer();

    connect(break_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));

    connect(root, SIGNAL(loadsrc()), this, SLOT(loadsrc()));
    connect(root, SIGNAL(delFile()), this, SLOT(deleteFile()));
    connect(root, SIGNAL(copyFile()), this, SLOT(copyFile()));
    connect(root, SIGNAL(loadWordList()), this, SLOT(loadWordList()));
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(root, SIGNAL(setTotalCount(int)),
            this, SLOT(setTotalcount(int)));
    connect(root, SIGNAL(setVerifier(int)),
            this, SLOT(setVerifier(int)));
    connect(root, SIGNAL(setStatus(int)),
            this, SLOT(setStatus(int)));
    connect(root, SIGNAL(saveWordList(QString)), this,
            SLOT(saveWordList(QString)));
    connect(root, SIGNAL(setCategory(QString)),
            this, SLOT(setCategory(QString)));
    connect(root, SIGNAL(setDifWords(QString)),
            this, SLOT(setDifWords(QString)));
    connect(root, SIGNAL(setFocusWord(QString)),
            this, SLOT(setFocusWord(QString)));

    readQmlProperties();
    updateStat();

}

void AbScene::readQmlProperties()
{
    man->params.status = QQmlProperty::read(root, "ab_status").toInt();
    man->params.total_count = QQmlProperty::read(root, "ab_total_count").toInt();
    man->params.num_words = QQmlProperty::read(root, "ab_num_words").toInt();
    man->params.count = QQmlProperty::read(root, "ab_count").toInt();
    man->params.verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    man->params.rec_time = QQmlProperty::read(root, "ab_rec_time").toFloat();
    man->params.pause_time = QQmlProperty::read(root, "ab_pause_time").toFloat();
    man->params.focus_word = QQmlProperty::read(root, "ab_focus_word").toString();
    man->params.category = QQmlProperty::read(root, "ab_category").toString();
}

void AbScene::setStatus(int status)
{
    man->params.status = status;
    QQmlProperty::write(root, "ab_status", status);
    if( status==AB_STATUS_REC && man->params.verifier==0 )
    {
        man->record();
    }
}

void AbScene::setVerifier(int verifier)
{
    man->params.verifier = verifier;
    man->swapParams();
    updateStat();
    setCount(0);

    if( verifier )
    {
        unverified_list = ab_listFiles(KAL_AU_DIR_WIN"unverified\\",
                                       AB_LIST_PATHS);
        man->params.total_count = unverified_list.size();
        QQmlProperty::write(root, "ab_total_count", unverified_list.size());
    }
}

void AbScene::setTotalcount(int val)
{
    man->params.total_count = val;
}

void AbScene::loadsrc()
{
    if( man->params.verifier==1 )
    {
        if( man->params.count<man->params.total_count )
        {
            setStatus(AB_STATUS_BREAK);
            loadAddress();
            setCount(man->params.count+1);
            break_timer->start(man->params.pause_time*1000);
        }
        else // cnt>=total
        {
            setStatus(AB_STATUS_STOP);
        }
    }
}

void AbScene::setCount(int cnt)
{
    man->params.count = cnt;
    QQmlProperty::write(root, "ab_count", man->params.count);
}

void AbScene::loadAddress()
{
    QString address = unverified_list[man->params.count];
    man->readWave(address);
    QQmlProperty::write(root, "ab_address", address);
}

void AbScene::deleteFile()
{
    QFile file(unverified_list[man->params.count-1]);
    file.remove();
}

void AbScene::copyFile()
{
    man->copyToOnline(unverified_list[man->params.count-1]);
}

void AbScene::setCategory(QString cat)
{
    man->params.category = cat;
    updateStat();
}

void AbScene::setFocusWord(QString focus_word)
{
    man->params.focus_word = focus_word;
}

void AbScene::updateStat()
{
    loadWordList();
    QString stat = ab_getStat(man->params.category);
    QQmlProperty::write(root, "ab_word_stat", stat);
    QString meanvar = ab_getMeanVar();
    QQmlProperty::write(root, "ab_mean_var", meanvar);
    updateCategories();
}

void AbScene::loadWordList()
{
    man->params.word_list = man->readWordList();
    QQmlProperty::write(root, "ab_word_list", man->params.word_list);
}

void AbScene::saveWordList(QString word_list)
{
    man->params.word_list = word_list;
    man->writeWordList();
}

void AbScene::setDifWords(QString difwords)
{
    man->params.dif_words = difwords;
    man->delWordSamples();
    updateStat();
}

void AbScene::breakTimeout()
{
    setStatus(AB_STATUS_PLAY);
    int playkon = QQmlProperty::read(root, "ab_playkon").toInt();
    if( playkon )
    {
        QQmlProperty::write(root, "ab_playkon", 0);
    }
    else
    {
        QQmlProperty::write(root, "ab_playkon", 1);
    }
    break_timer->stop();
}

void AbScene::processKey(int key)
{
    if( key==Qt::Key_T )
    {
        QString path = QDir::currentPath() + "\\";
        path.replace('/', '\\');
        path +=  KAL_AU_DIR_WIN"train\\";
        path += man->params.category + "\\";
        QString cmd = "explorer.exe " + path;
        system(cmd.toStdString().c_str());
    }
    else if( key==Qt::Key_K )
    {
        float rec_time = QQmlProperty::read(root,
                            "ab_rec_time").toFloat();
        rec_time += .1;
        man->params.rec_time = rec_time;
        QQmlProperty::write(root, "ab_rec_time", rec_time);
    }
    else if( key==Qt::Key_J )
    {
        float rec_time = QQmlProperty::read(root,
                            "ab_rec_time").toFloat();
        rec_time -= .1;
        man->params.rec_time = rec_time;
        QQmlProperty::write(root, "ab_rec_time", rec_time);
    }
    else if( key==Qt::Key_Up )
    {
        float pause_time = QQmlProperty::read(root,
                            "ab_pause_time").toFloat();
        pause_time += .1;
        man->params.pause_time = pause_time;
        QQmlProperty::write(root, "ab_pause_time", pause_time);
    }
    else if( key==Qt::Key_Down )
    {
        float pause_time = QQmlProperty::read(root,
                            "ab_pause_time").toFloat();
        pause_time -= .1;
        man->params.pause_time = pause_time;
        QQmlProperty::write(root, "ab_pause_time", pause_time);
    }
    else if( key==Qt::Key_Left )
    {
        int num_words = QQmlProperty::read(root,
                            "ab_num_words").toInt();
        num_words--;
        man->params.num_words = num_words;
        QQmlProperty::write(root, "ab_num_words", num_words);
    }
    else if( key==Qt::Key_Right )
    {
        int num_words = QQmlProperty::read(root,
                            "ab_num_words").toInt();
        num_words++;
        man->params.num_words = num_words;
        QQmlProperty::write(root, "ab_num_words", num_words);
    }
    else
    {
        return;
    }
}

void AbScene::updateCategories()
{
    QFileInfoList dir_list = ab_getAudioDirs();
    QStringList categories;
    int len = dir_list.size();
    for( int i=0 ; i<len ; i++)
    {
        categories.append(dir_list[i].fileName());
    }
    QString cat_str = categories.join("!");
    QQmlProperty::write(root, "ab_auto_comp", cat_str);
}
