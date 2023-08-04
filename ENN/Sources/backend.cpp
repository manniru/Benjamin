#include "backend.h"
#include <QDir>
#include <QDataStream>

clock_t bt_last_clock;
QString wsl_path = "";

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

QStringList enn_listDirs(QString path)
{
    QDir p_dir(path);
    QStringList fmt;
    fmt.append("*");
    QStringList dir_list = p_dir.entryList(fmt, QDir::Dirs | QDir::NoDotAndDotDot);

    return dir_list;
}

QStringList enn_listImages(QString path)
{
    QDir p_dir(path);
    QStringList fmt;
    fmt.append("*.png");
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    return file_list;
}

QStringList enn_listDatas(QString path)
{
    QDir p_dir(path);
    QStringList fmt;
    fmt.append("*.enn");
    QStringList file_list = p_dir.entryList(fmt, QDir::Files);

    return file_list;
}

void enn_readENN(QString path, tiny_dnn::vec_t *out)
{
    QFile m_file(path);

    m_file.open(QIODevice::ReadOnly);
    QDataStream in(&m_file);
    qint32 width;
    qint32 height;
    in >> width;
    in >> height;
//        qDebug() << data_path << width << height;
    float val;
    out->resize(width*height);

    for( int j=0 ; j<height ; j++ )
    {
        for( int k=0 ; k<width ; k++ )
        {
            in >> val;
//            printf("%5.1f ", val);
            (*out)[j*height+k] = val;
        }
//        printf("\n");
    }
    m_file.close();
}

QStringList bt_parseLexicon(QString filename)
{
    QFile words_file(filename);
    QStringList lexicon;

    if( !words_file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Error opening" << filename;
        return lexicon;
    }

    while ( !words_file.atEnd() )
    {
        QString line = QString(words_file.readLine());
        line.remove("\n");
        QStringList line_list = line.split(" ");
        lexicon.append(line_list[0]);
    }

    words_file.close();
    return lexicon;
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
                wsl_path.replace("/", "\\");
                return wsl_path;
            }
        }
    }

    return "";
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
        ret.push_back(file_list[i].fileName());
    }

    return ret;
}

QString ab_getAudioPath()
{
#ifdef WIN32
    QString audio_path = wsl_path;
    if( audio_path.isEmpty() )
    {
        audio_path = ab_getWslPath();
    }
    audio_path += "\\Benjamin\\Nato\\audio\\";
    return audio_path;
#else
    return KAL_AU_DIR;
#endif
}
