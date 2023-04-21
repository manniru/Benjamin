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
    qDebug() << "batool_dir" << batool_dir;
    enn_console->startConsole(batool_dir);
}

AbTrain::~AbTrain()
{
    delete enn_console;
    delete console;
    enn_thread->exit();
    delete enn_thread;
    con_thread->exit();
    delete con_thread;
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
        new_path +=  "\\" + files[i];

        QFile file(old_path);
        if( QFile::exists(new_path) )
        {
            QFile::remove(new_path);
        }
        file.copy(new_path);
    }
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
    qDebug() << "train KalB";
    QQmlProperty::write(console_qml, "visible", true);

    int test_count = needTestCount();
    if( test_count )
    {
        addTestSample(test_count);
    }
    console->wsl_run("./wsl_init.sh");
    console->wsl_run("./wsl_train.sh");
}

void AbTrain::createENN()
{
    qDebug() << "createENN";
    QQmlProperty::write(console_qml, "visible", true);
    enn_console->prompt = "Tool>";
    enn_console->run("cd ..\\Tool");
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
    QString model_path = AB_MODEL_DIR;
    QDir model_dir(model_path);

    if( !model_dir.exists() )
    {
        qDebug() << "Creating" << model_path
                 << " Directory";
#ifdef WIN32
        QString cmd = "mkdir " + model_path;
        system(cmd.toStdString().c_str());
#else //OR __linux
        system("mkdir -p ../Tools/Model");
#endif
    }
}

int AbTrain::needTestCount()
{
    int sample_count = 0;
    int test_need_count;
    int test_curr_count;
    int ret;

    sample_count = getTrainCount();
    test_need_count = sample_count/10;
    test_curr_count = getTestCount();
    ret = test_need_count-test_curr_count ;

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
        int cat_id = 1+rand()%cat_count;
        int cat_len = stat->cache_files[cat_id].size();
        int sample_id = rand()%cat_len;
        QString sample_path = stat->cache_files[cat_id][sample_id];

        QFile file(sample_path);
        QFileInfo info(sample_path);
        QString new_path = test_path+info.fileName();
        file.copy(new_path);
        file.remove();
        stat->cache_files[cat_id].remove(sample_id);
    }
}

int AbTrain::getTestCount()
{
    QString test_path = ab_getAudioPath();
    test_path += "test\\";
    QDir dir_test(test_path);
    if( !dir_test.exists() )
    {
#ifdef WIN32
        QString cmd = "mkdir " + test_path;
        system(cmd.toStdString().c_str());
#else //OR __linux
        system("mkdir -p " KAL_AU_DIR "test");
#endif
        qDebug() << "Info: Directory" << test_path << "Created";
    }

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
        ret += stat->cache_files[i+1].size();
    }

    return ret;
}
