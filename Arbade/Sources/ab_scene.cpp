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
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(root, SIGNAL(setVerifier(int)),
            this, SLOT(setVerifier(int)));
    connect(root, SIGNAL(setStatus(int)),
            this, SLOT(setStatus(int)));
    connect(root, SIGNAL(saveWordList()), this,
            SLOT(saveWordList()));
    connect(root, SIGNAL(setCategory()),
            this, SLOT(setCategory()));
    connect(root, SIGNAL(setDifWords()),
            this, SLOT(setDifWords()));
    connect(root, SIGNAL(setFocusWord(int)),
            this, SLOT(setFocusWord(int)));

    readQmlProperties();
    updateStat();
}

void AbScene::readQmlProperties()
{
    setVerifier(QQmlProperty::read(root, "ab_verifier").toInt());
    setFocusWord(QQmlProperty::read(root, "ab_focus_word").toInt());
}

void AbScene::setStatus(int status)
{
    QQmlProperty::write(root, "ab_status", status);
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();

    if( status==AB_STATUS_REC && verifier==0 )
    {
        man->record();
    }
    else if( status==AB_STATUS_PAUSE && verifier==0 )
    {
        updateStat();
    }
}

void AbScene::setVerifier(int verifier)
{
    updateStat();
    setCount(0);

    if( verifier )
    {
        QString unverified_dir = ab_getAudioPath();
        unverified_dir += "unverified\\";
        unverified_list = ab_listFiles(unverified_dir, AB_LIST_PATHS);
        QQmlProperty::write(root, "ab_total_count_v", unverified_list.size());
    }
}

void AbScene::loadsrc()
{
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    if( verifier==1 )
    {
        int total_count = QQmlProperty::read(root, "ab_total_count_v").toInt();
        int count = QQmlProperty::read(root, "ab_count").toInt();
        if( count<total_count )
        {
            setStatus(AB_STATUS_BREAK);
            loadAddress();
            setCount(count+1);
            float pause_time = QQmlProperty::read(root, "ab_pause_time").toFloat();
            break_timer->start(pause_time*1000);
        }
        else // cnt>=total
        {
            setStatus(AB_STATUS_STOP);
        }
    }
}

void AbScene::setCount(int cnt)
{
    QQmlProperty::write(root, "ab_count", cnt);
}

void AbScene::loadAddress()
{
    int count = QQmlProperty::read(root, "ab_count").toInt();
    QString address = unverified_list[count];
    man->readWave(address);
    QQmlProperty::write(root, "ab_address", address);
}

void AbScene::deleteFile()
{
    int count = QQmlProperty::read(root, "ab_count").toInt();
    QFile file(unverified_list[count-1]);
    file.remove();
}

void AbScene::copyFile()
{
    int count = QQmlProperty::read(root, "ab_count").toInt();
    man->copyToOnline(unverified_list[count-1]);
}

void AbScene::setCategory()
{
    updateStat();
}

void AbScene::setFocusWord(int focus_word)
{
    QString focus_text;
    if( focus_word==-1 )
    {
        focus_text = "<empty>";
    }
    else
    {
        focus_text = man->idToWords(focus_word);
    }
    QQmlProperty::write(root, "ab_focus_text", focus_text);
}

void AbScene::updateStat()
{
    loadWordList();
    QString stat;

    if( !catmode )
    {
        QString category = QQmlProperty::read(root, "ab_category").toString();
        stat = ab_getStat(category);
//        qDebug() << "updateStat" << category;
    }
    else
    {
        stat = ab_getStat();
    }
    QQmlProperty::write(root, "ab_word_stat", stat);
    QString meanvar = ab_getMeanVar();
    QQmlProperty::write(root, "ab_mean_var", meanvar);
//    qDebug() << "updateStat" << meanvar << stat;
    updateCategories();
}

void AbScene::loadWordList()
{
    QString word_list = man->readWordList();
    QQmlProperty::write(root, "ab_word_list", word_list);
}

void AbScene::saveWordList()
{
    man->writeWordList();
}

void AbScene::setDifWords()
{
    man->delWordSamples();
    updateStat();
}

void AbScene::breakTimeout()
{
    setStatus(AB_STATUS_PLAY);
    QMetaObject::invokeMethod(root, "playkon");
    break_timer->stop();
}

void AbScene::processKey(int key)
{
    if( key==Qt::Key_O )
    {
        QString category = QQmlProperty::read(root, "ab_category").toString();
        ab_openCategory(category);
    }
    else if( key==Qt::Key_W )
    {
        catmode = !catmode;
        updateStat();
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
