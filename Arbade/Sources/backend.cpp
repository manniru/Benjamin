#include "backend.h"
#include "config.h"

QFile *log_file = NULL;
clock_t bt_last_clock;

int getIntCommand(char *command)
{
    FILE *fp;
    int returnData;

    char path[1035];

    /* Open the command for reading. */
    fp = popen(command, "r");

    if( fp==NULL)
    {
        printf("Failed to run command\n" );
        return -1;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path)-1, fp)!=NULL)
    {
        returnData = atoi(path);
    }

    /* close */
    pclose(fp);
    return returnData;
}

QString getStrCommand(QString command)
{
    FILE *fp;
    QString returnData;

    char path[1035];

    /* Open the command for reading. */
    fp = popen(command.toStdString().c_str(), "r");

    if( fp==NULL )
    {
        printf("Failed to run command\n");
        return returnData;
    }

    /* Read the output a line at a time - output it. */
    while( fgets(path, sizeof(path)-1, fp)!=NULL )
    {
        returnData += QString(path);
    }

    // Remove last \n
    returnData.remove(returnData.length()-1, 1);

    /* close */
    pclose(fp);
    return returnData;
}

QString getDiffTime(clock_t start)
{
    QString ret;
    clock_t end = clock();
    bt_last_clock = end;
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    ret  = QString::number(qRound(cpu_time_used*1000));
    ret += "ms";
    return  ret;
}

// compare with last
QString getLDiffTime()
{
    QString ret;
    clock_t end = clock();
    double cpu_time_used = ((double) (end - bt_last_clock)) / CLOCKS_PER_SEC;
    ret  = QString::number(qRound(cpu_time_used*1000));
    ret += "ms";
    bt_last_clock = end;
    return  ret;
}

QStringList bt_parseLexicon()
{
    QString filename = ab_getAudioPath() + "..\\word_list";
    QFile words_file(filename);
    QStringList lexicon;

    if( !words_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << filename;
        return lexicon;
    }

    while( !words_file.atEnd() )
    {
        QString line = QString(words_file.readLine());
        line = line.trimmed();
        QStringList line_list = line.split(" ");
        lexicon.append(line_list[0]);
    }

    words_file.close();
    return lexicon;
}

void bt_addLog(QString log)
{
    if( log_file==NULL )
    {
        log_file = new QFile("out_log");

        if( !log_file->open(QIODevice::WriteOnly) )
        {
            qDebug() << "Failed To Create out_log";
            exit(1);
        }
    }

    log_file->write(log.toStdString().c_str());
    log_file->write("\n");
}

void bt_mkDir(QString path)
{
    QDir au_Dir(path);

    if( !au_Dir.exists() )
    {
//        qDebug() << "Creating" << path << " Directory";
        QString command;
#ifdef WIN32
        command = "mkdir ";
#else //OR __linux
        command = "mkdir -p ";
#endif
        command += path;
        system( command.toStdString().c_str() );
    }
}

QString ab_getWslPath()
{
    QFileInfoList drives = QDir::drives();
    int len = drives.size();
    for( int i=0 ; i<len ; i++ )
    {
        QDir arch_dir(drives[i].filePath());
        QFileInfoList dir_list = arch_dir.entryInfoList(QDir::Dirs);
        int dirs_num = dir_list.size();
        for( int j=0 ; j<dirs_num ; j++ )
        {
            if( dir_list[j].fileName()==AB_WSL_ROOT )
            {
                QString wsl_path = dir_list[j].filePath();
                wsl_path = correctWinPath(wsl_path);
                return wsl_path;
            }
        }
    }

    return "";
}

QString ab_getAudioPath()
{
#ifdef WIN32
    QString audio_path = ab_getWslPath();
    audio_path += "\\Benjamin\\Nato\\audio\\";
    return audio_path;
#else
    return KAL_AU_DIR;
#endif
}

QVector<QString> ab_listFiles(QString path)
{
    QVector<QString> ret;
    QDir cat_dir(path);

    if( !cat_dir.exists() )
    {
        qDebug() << "Warning: Directory doesn't Exist,"
                 << "cannot generate statistics.";
        return ret;
    }

    QFileInfoList file_list = cat_dir.entryInfoList(QDir::Files,
                              QDir::Time | QDir::Reversed);

    int len = file_list.size();

    for( int i=0 ; i<len ; i++ )
    {
        ret.push_back(file_list[i].absoluteFilePath());
    }

    return ret;
}

QStringList ab_listFilesSorted(QString path)
{
    QStringList ret;
    QDir cat_dir(path);
    QFileInfoList file_list = cat_dir.entryInfoList(QDir::Files,
                             QDir::Time | QDir::Reversed);
    int len = file_list.length();
    for( int i=0 ; i<len ; i++ )
    {
        ret.push_back(file_list[i].absolutePath());
    }

    return ret;
}

void ab_openCategory(QString category)
{
    QString path = ab_getAudioPath();

    if( category!=AB_UNVER_DIR && category!=AB_SHIT_DIR )
    {
        path += "train";
        path += QDir::separator();
    }

    path += category + QDir::separator();
    qDebug() << "cat" << category << path;

    QString cmd = "explorer.exe " + path;
    system(cmd.toStdString().c_str());
}

QFileInfoList ab_getAudioDirs()
{
    QFileInfoList dir_list;
    QString audio_path = ab_getAudioPath();
    QString path = audio_path + "unverified";
    QFileInfo unver(path);
    if( !unver.exists() )
    {
        qDebug() << "Warning: unverified directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }
    dir_list.append(unver); // add unverified dir to list of train category dirs

    path = audio_path + AB_SHIT_DIR;
    unver.setFile(path);
    if( !unver.exists() )
    {
        qDebug() << "Warning: shit directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }
    dir_list.append(unver); // add shit dir to list of train category dirs

    path = audio_path + "train\\";
    QDir dir(path);

    if( !dir.exists() )
    {
        qDebug() << "Warning: train directory doesn't Exist,"
                 << "cannot generate statistics.";
        return QFileInfoList();
    }

    dir_list += dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    return dir_list;
}

QString getStatusStr(int status)
{
    if( status==AB_STATUS_REC )
    {
        return "Record";
    }
    else if( status==AB_STATUS_PAUSE )
    {
        return "Pause";
    }
    else if( status==AB_STATUS_STOP )
    {
        return "Stop";
    }
    else if( status==AB_STATUS_REQPAUSE )
    {
        return "Req Pause";
    }
    else if( status==AB_STATUS_BREAK )
    {
        return "Break";
    }
    else if( status==AB_STATUS_PLAY )
    {
        return "Play";
    }
    return "";
}

// used for debuging purposes
QString getVerifierStr(int verifier)
{
    if( verifier==0 )
    {
        return "Recorder";
    }
    else if( verifier==1 )
    {
        return "verifier";
    }
    else if( verifier==2 )
    {
        return "shit mode";
    }
    return "";
}

// check if dirname does not exist in audio directory then create one
int ab_checkAuDir(QString dirname)
{
    QString path = ab_getAudioPath() + dirname;
    return ab_checkDir(path);
}

// check if path does not exist create one
int ab_checkDir(QString path)
{
    QDir dir(path);

    if( !dir.exists() )
    {
        qDebug() << "Creating" << path
                 << " Directory";
#ifdef WIN32
        QString cmd = "mkdir " + path;
        system(cmd.toStdString().c_str());
#else //OR __linux
        QString cmd = "mkdir -p ";
        cmd += path;
        system(cmd.toStdString().c_str());
#endif
        return 0;
    }
    return 1;
}

QString correctWinPath(QString path)
{
#ifdef WIN32
    path.replace("/", "\\");
#endif
    return path;
}
