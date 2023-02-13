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
    connect(root, SIGNAL(setCategory(QString)),
            this, SLOT(setCategory(QString)));
    connect(root, SIGNAL(setDifWords(QString)),
            this, SLOT(setDifWords(QString)));
    connect(root, SIGNAL(setFocusWord(QString)),
            this, SLOT(setFocusWord(QString)));
    connect(root, SIGNAL(setVerifier(int)),
            this, SLOT(setVerifier(int)));
    connect(root, SIGNAL(setStatus(int)),
            this, SLOT(setVerifier(int)));

    updateStat();
}

void AbScene::setStatus(int status)
{
    man->params.status = status;

    if( status==AB_STATUS_REC && man->params.verifier==0 )
    {
        man->record();
    }
    QQmlProperty::write(root, "ab_status", status);
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
    QString stat = ab_getStat(man->params.category);
    QQmlProperty::write(root, "ab_stat", stat);
    QString meanvar = ab_getMeanVar();
    QQmlProperty::write(root, "ab_mean_var", meanvar);
    updateCategories();
}

void AbScene::loadWordList()
{
    man->params.word_list = man->readWordList();
    QQmlProperty::write(root, "ab_word_list", man->params.word_list);
    QString word_stat = ab_getStat();
    QQmlProperty::write(root, "ab_word_stat", word_stat);
}

void AbScene::saveWordlist(QString word_list)
{
    man->params.word_list = word_list;
    man->writeWordList();
}

void AbScene::setDifWords(QString difwords)
{
    man->params.dif_words = difwords;
    man->delWordSamples();
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
    setAutocomp(cat_str);
}

void ab_setUi(QObject *ui)
{
    root = ui;
}

QObject* ab_getUi()
{
    return root;
}
