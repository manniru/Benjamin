#include "ab_scene.h"
#include <QQmlProperty>
#include <QGuiApplication>

AbScene::AbScene(QObject *ui, QObject *parent) : QObject(parent)
{
    root = ui;
    qml_editor = root->findChild<QObject *>("WordList");
    message = root->findChild<QObject *>("Message");

    break_timer = new QTimer();
    editor = new AbEditor(root);

    audio = new AbAudio(editor->stat, root);

    connect(audio, SIGNAL(setStatus(int)),
            this, SLOT(updateStatus(int)));

    connect(break_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));

    connect(root, SIGNAL(startPauseV()), this, SLOT(startPauseV()));
    connect(root, SIGNAL(delFile()), this, SLOT(deleteFile()));
    connect(root, SIGNAL(copyUnverifyFile()), this, SLOT(copyUnverifyFile()));
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(root, SIGNAL(verifierChanged()),
            this, SLOT(verifierChanged()));
    connect(root, SIGNAL(setStatus(int)),
            this, SLOT(setStatus(int)));
    connect(root, SIGNAL(setCategory()),
            this, SLOT(setCategory()));
    connect(root, SIGNAL(setDifWords()),
            this, SLOT(setDifWords()));
    connect(root, SIGNAL(setFocusWord(int)),
            this, SLOT(setFocusWord(int)));
    connect(editor->stat, SIGNAL(cacheCreated()),
            this, SLOT(cacheCreated()));

    updateAutoCpmplete();
    createEditor();
}

void AbScene::readQmlProperties()
{
    verifierChanged();
    setFocusWord(QQmlProperty::read(root, "ab_focus_word").toInt());
}

void AbScene::createEditor()
{
    editor->createList();
}

void AbScene::setStatus(int status)
{
    QQmlProperty::write(root, "ab_status", status);
    qDebug() << "setStatus" << status;
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();

    if( status==AB_STATUS_REC && verifier==0 )
    {
        audio->record();
    }
    else if( status==AB_STATUS_PAUSE && verifier==0 )
    {
        editor->updateStat();
    }
    else if( status==AB_STATUS_STOP )
    {
        editor->updateStat();
    }
}

void AbScene::updateStatus(int status)
{
    QQmlProperty::write(root, "ab_status", status);
    if( status==AB_STATUS_STOP )
    {
        editor->updateStat();
    }
}

void AbScene::verifierChanged()
{
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    editor->updateStat();
    if( verifier )
    {
        int count = editor->stat->cache_files[0].size();
        QQmlProperty::write(root, "ab_total_count_v", count);
    }
}

// start pause timer before playing out sample
// only runs in verification mode
void AbScene::startPauseV()
{
    int total_count = QQmlProperty::read(root, "ab_total_count_v").toInt();

    if( total_count==0 )
    {
        QString msg = "There is no sample in the unverified directory";
        QQmlProperty::write(message, "message", msg);
    }

    int count = QQmlProperty::read(root, "ab_count").toInt();

    if( count<total_count )
    {
        setStatus(AB_STATUS_BREAK);
        loadAddress();
        setCount(count + 1);
        float pause_time = QQmlProperty::read(root, "ab_verify_pause").toFloat();
        break_timer->start(pause_time * 1000);
    }
    else // cnt>=total
    {
        setStatus(AB_STATUS_STOP);
    }
}

void AbScene::setCount(int cnt)
{
    QQmlProperty::write(root, "ab_count", cnt);
}

void AbScene::loadAddress()
{
    int count = QQmlProperty::read(root, "ab_count").toInt();
    QString address = editor->stat->cache_files[0][count];
    qDebug() << "address = " << address;
    audio->updateAudioParam(address);
    QQmlProperty::write(root, "ab_address", address);
}

void AbScene::deleteFile()
{
    int count = QQmlProperty::read(root, "ab_count").toInt();
    QFile file(editor->stat->cache_files[0][count-1]);
    file.remove();
    editor->stat->cache_files[0].remove(count-1);
}

void AbScene::copyUnverifyFile()
{
//    int count = QQmlProperty::read(root, "ab_count").toInt();
//    editor->stat->copyToOnline(editor->stat->cache_files[0][count-1]);
}

void AbScene::setCategory()
{
    editor->updateStat();
    updateAutoCpmplete();
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
        focus_text = editor->stat->idToWord(focus_word);
    }
    QQmlProperty::write(root, "ab_focus_text", focus_text);
}

void AbScene::setDifWords()
{
    editor->stat->delWordSamples();
    editor->updateStat();
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
        QString category = QQmlProperty::read(qml_editor, "category").toString();
        ab_openCategory(category);
    }
    else if( key==Qt::Key_W )
    {
        editor->updateStat();
    }
    else
    {
        return;
    }
}

void AbScene::updateAutoCpmplete()
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

void AbScene::cacheCreated()
{
    readQmlProperties();
}
