#include "ab_train.h"
#include <QDebug>
#include <QQmlProperty>

AbTrain::AbTrain(AbStat *st, QObject *ui, QObject *parent) : QObject(parent)
{
    stat = st;
    wsl = new AbInitWSL();
    wsl_thread = new QThread();
    wsl->moveToThread(wsl_thread);

    wsl_thread->start();
    root = ui;
    topbar = root->findChild<QObject*>("TopBar");
    wsl_dialog = root->findChild<QObject*>("WslDialog");
    console_qml = root->findChild<QObject*>("Console");
    connect(root, SIGNAL(sendKey(int)), this, SLOT(processKey(int)));
    connect(wsl_dialog, SIGNAL(driveEntered(QString)),
            wsl, SLOT(createWSL(QString)));

    console = new AbConsole(root);
    enn_console = new AbConsole(root);

    connect(console, SIGNAL(finished()), this, SLOT(trainFinished()));

    connect(this, SIGNAL(createWSL(QString)),
            wsl, SLOT(createWSL(QString)));
    connect(wsl, SIGNAL(WslCreated()),
            this, SLOT(WslCreated()));

    QString batool_dir = QDir::currentPath();
    batool_dir += "/../Tools";
    enn_console->startConsole(batool_dir);
    checkBenjamin();
    updateWerSer();
}

AbTrain::~AbTrain()
{
    enn_thread->exit();
    con_thread->exit();
    delete enn_console;
    delete console;
}

void AbTrain::processKey(int key)
{
    if( key==Qt::Key_T )
    {
        train();
    }
    else if( key==Qt::Key_Escape )
    {
        ;
    }
    else if( key==Qt::Key_D )
    {
        createENN();
    }
    else if( key==Qt::Key_E )
    {
        trainENN();
    }
    else if( key==Qt::Key_B )
    {

    }
}

void AbTrain::WslCreated()
{
    console->startConsole(wsl_path);
}

void AbTrain::trainFinished()
{
    qDebug() << QDir::currentPath();
    checkModelExist();

    QVector<QString> files;

    files << "HCLG.fst";
    files << "words.txt";
    files << "final.oalimdl";
    files << "global_cmvn.stats";

    int len = files.length();
    for( int i=0 ; i<len ; i++ )
    {
        QString old_path = wsl_path + "\\Benjamin\\Tools\\Model\\";
        old_path += files[i];
        QString new_path = AB_MODEL_DIR;
        new_path +=  QDir::separator() + files[i];

        QFile file(old_path);
        if( QFile::exists(new_path) )
        {
            QFile::remove(new_path);
        }
        file.copy(new_path);
    }

    updateWerSer();
    checkOnlineExist(); // we should remake online dir
                        // if it was deleted because of emptiness
}

// called from main
void AbTrain::initWsl()
{
    wsl_path = ab_getWslPath();
    if( wsl_path.isEmpty() )
    {
        qDebug() << "Error 152: shit has happened";
        exit(52);
    }
    if( !QFile::exists(wsl_path + "\\ext4.vhdx") )
    {
        QString drive = QString(wsl_path[0]);
        emit createWSL(drive);
    }
    else
    {
        console->startConsole(wsl_path);
    }
}

void AbTrain::train()
{
    qDebug() << "train KalB" << wsl_path;
    QQmlProperty::write(console_qml, "visible", true);

    int test_count = needTestCount();
    if( test_count )
    {
        addTestSample(test_count);
    }
    removeEmptyDirs(); // empty dirs should be removed for Kaldi
    console->wsl_run("./wsl_init.sh");
    console->wsl_run("./wsl_train.sh");
}

void AbTrain::createENN()
{
    qDebug() << "createENN";
    QQmlProperty::write(console_qml, "visible", true);
    enn_console->prompt = "Tools>";
    enn_console->run("cd ..\\Tools");
    enn_console->run("release\\BaTool.exe e");
}

void AbTrain::trainENN()
{
    qDebug() << "createENN";
    QQmlProperty::write(console_qml, "visible", true);
    enn_console->prompt = "ENN>";
    enn_console->run("cd ..\\ENN");
    enn_console->run("release\\ENN.exe");
}

void AbTrain::checkModelExist()
{
    QString model_path;
#ifdef WIN32
    model_path = AB_MODEL_DIR;
#else //OR __linux
    model_path = "../Tools/Model";
#endif
    ab_checkDir(model_path);
}

int AbTrain::needTestCount()
{
    float sample_count = 0;
    float percent = AB_TEST_PERCENT/100.0;
    int   test_need_count;
    int   test_curr_count;
    int   ret;

    sample_count = getTrainCount();
    test_need_count = qFloor(sample_count*percent);
    test_curr_count = getTestCount();
    ret = test_need_count-test_curr_count;

    if( ret>0 )
    {
        return ret;
    }
    else
    {
        return 0;
    }
}

void AbTrain::addTestSample(int count)
{
    QString  train_path = ab_getAudioPath() + "train\\";
    QDir dir(train_path);
    QFileInfoList dir_list = dir.entryInfoList(QDir::Dirs
                                               | QDir::NoDotAndDotDot);
    QString test_path = ab_getAudioPath();
    test_path += "test\\";

    int cat_count = dir_list.size();

    for( int i=0 ; i<count ; i++ )
    {
        int cat_id = 2+rand()%cat_count;
        int cat_len = stat->cache->cache_files[cat_id].size();
        int sample_id = rand()%cat_len;
        QString sample_path = stat->cache->cache_files[cat_id][sample_id];

        QFile file(sample_path);
        QFileInfo info(sample_path);
        QString new_path = test_path+info.fileName();
        file.copy(new_path);
        file.remove();
        stat->cache->deleteCache(cat_id, sample_id);
    }
}

int AbTrain::getTestCount()
{
    ab_checkAuDir("test");
    QString test_path = ab_getAudioPath() + "test\\";

    QVector<QString> test_files = ab_listFiles(test_path);
    int count = test_files.count();

    return count;
}

int AbTrain::getTrainCount()
{
    QString  path = ab_getAudioPath() + "train\\";
    QDir dir(path);
    int ret = 0;
    QFileInfoList dir_list = dir.entryInfoList(QDir::Dirs
                                               | QDir::NoDotAndDotDot);
    int len_dir = dir_list.size();
    if( len_dir==0 )
    {
        return 0;
    }

    for( int i=0 ; i<len_dir ; i++ )
    {
        ret += stat->cache->cache_files[i+2].size();
    }

    return ret;
}

void AbTrain::removeEmptyDirs()
{
    QFileInfoList audio_dirs = ab_getAudioDirs();
    int len = audio_dirs.length();
    for( int i=2 ; i<len ; i++ )
    {
        QString cat_dirname = audio_dirs[i].absoluteFilePath();
        QDir cat_dir(cat_dirname);
        QFileInfoList file_list = cat_dir.entryInfoList(QDir::Files);
        if( file_list.length()==0 )
        {
            qDebug() << "Info:" << cat_dirname << "deleted because of being empty";
            cat_dir.removeRecursively();
        }
    }
}

void AbTrain::checkOnlineExist()
{
    QString dirname = "train";
    dirname += QDir::separator();
    dirname += "online";
    ab_checkAuDir(dirname);
}

void AbTrain::updateWerSer()
{
    QString wer_path;
#ifdef WIN32
    wer_path = ab_getWslPath();
    wer_path += "\\Benjamin\\Nato\\exp\\tri1\\decode\\wer_16";
#else
    wer_path = KAL_WER_DIR"wer_16";
#endif

    QFile wer_file(wer_path);
    if( !wer_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Warning 121: cannot open" << wer_path;
        return;
    }

    QString wer_content = wer_file.readAll();
    QString wer_text = wer_content.split("%WER")[1];
    QString ser_text = wer_content.split("%SER")[1];
    wer_text = wer_text.split("[")[0];
    ser_text = ser_text.split("[")[0];
    double wer = wer_text.trimmed().toDouble();
    double ser = ser_text.trimmed().toDouble();
    qDebug() << "updateWerSer" << wer << ser;
    QQmlProperty::write(topbar, "model_wer", wer);
    QQmlProperty::write(topbar, "model_ser", ser);
}

void AbTrain::checkBenjamin()
{
    QString path;
#ifdef WIN32
    wsl_path = ab_getWslPath();
    if( wsl_path.isEmpty() )
    {
        return;
    }
    console->startConsole(wsl_path);
    path = wsl_path + "\\Benjamin\\Tools\\";
#else
    path = KAL_TOOL_DIR;
#endif
    QDir dir(path);
    if( !dir.exists() )
    {
        console->wsl_run("./wsl_init.sh");
    }
}
