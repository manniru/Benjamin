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
    verify = new AbVerify(editor, root);

    connect(audio, SIGNAL(setStatus(int)),
            this, SLOT(setStatusAudio(int)));

    connect(break_timer, SIGNAL(timeout()),
            this, SLOT(breakTimeout()));

    connect(root, SIGNAL(startPauseV()), this, SLOT(startPauseV()));
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(root, SIGNAL(verifierChanged()),
            this, SLOT(verifierChanged()));
    connect(root, SIGNAL(setStatus(int)),
            this, SLOT(setStatus(int)));
    connect(root, SIGNAL(setCategory()),
            this, SLOT(setCategory()));
    connect(root, SIGNAL(focusWordChanged()),
            this, SLOT(focusWordChanged()));
    connect(editor->stat, SIGNAL(cacheCreated()),
            this, SLOT(cacheCreated()));

    updateAutoCpmplete();
    createEditor();
}

void AbScene::createEditor()
{
    editor->createList();
}

// This function is called both from C++ and QML
void AbScene::setStatus(int status)
{
    QQmlProperty::write(root, "ab_status", status);
    qDebug() << "setStatus" << getStatusStr(status);
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();

    if( status==AB_STATUS_REC && verifier==0 )
    {
        audio->record();
    }
    else if( status==AB_STATUS_STOP && verifier==0 )
    {
        audio->stop();
    }
    else if( status==AB_STATUS_STOP && verifier==1 )
    {
        editor->clearRecList();
        editor->updateStat();
    }
}

void AbScene::setStatusAudio(int status)
{
    QQmlProperty::write(root, "ab_status", status);
    if( status==AB_STATUS_STOP )
    {
        QQmlProperty::write(root, "ab_count", 0);
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
        if( count>AB_MAX_RECLIST )
        {
            count = AB_MAX_RECLIST;
        }
        QQmlProperty::write(root, "ab_total_count_v", count);
    }

    editor->clearRecList();
    focusWordChanged();
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
        count++;
        setStatus(AB_STATUS_BREAK);
        loadVerifyFile();
        QQmlProperty::write(root, "ab_count", count);
        float pause_time = QQmlProperty::read(root, "ab_verify_pause").toFloat();
        break_timer->start(pause_time * 1000);
    }
    else // cnt>=total
    {
        setStatus(AB_STATUS_STOP);
    }
}

void AbScene::loadVerifyFile()
{
    QString file_path = editor->stat->cache_files[0].last();
    audio->updateVerifyParam(file_path);
    QQmlProperty::write(root, "ab_address", file_path);
}

void AbScene::setCategory()
{
    editor->updateStat();
    updateAutoCpmplete();
}

void AbScene::focusWordChanged()
{
    QString focus_text;
    int focus_word;

    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    if( verifier )
    {
        focus_word = QQmlProperty::read(root, "ab_focus_word_v").toInt();
    }
    else
    {
        focus_word = QQmlProperty::read(root, "ab_focus_word").toInt();
    }
    if( focus_word==-1 )
    {
        focus_text = "<empty>";
    }
    else
    {
        focus_text = editor->stat->idToWord(focus_word);
    }
    if( verifier )
    {
        QQmlProperty::write(root, "ab_focus_text_v", focus_text);
    }
    else
    {
        QQmlProperty::write(root, "ab_focus_text", focus_text);
    }
}

void AbScene::breakTimeout()
{
    int status = QQmlProperty::read(root, "ab_status").toInt();
    if( status==AB_STATUS_BREAK )
    {
        setStatus(AB_STATUS_PLAY);
        QMetaObject::invokeMethod(root, "playkon");
        break_timer->stop();
    }
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
        // toggle all stat
        int stat = QQmlProperty::read(root, "ab_all_stat").toInt();
        QQmlProperty::write(root, "ab_all_stat", !stat);
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
    int verifier = QQmlProperty::read(root, "ab_verifier").toInt();
    if( verifier )
    {
        int count = editor->stat->cache_files[0].size();
        if( count>AB_MAX_RECLIST )
        {
            count = AB_MAX_RECLIST;
        }
        QQmlProperty::write(root, "ab_total_count_v", count);
    }
    focusWordChanged();
}
