#include "backend.h"
#include "config.h"
#include <QFile>
#include <QDir>

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

float enn_getDimScale(double p, int len)
{
    int gf = ENN_GAURD_TIME * 100; // gaurd_frame = 5
    double gs_step = 1.0/(gf+1); // gaurd scale step
    double ds = 1; //dim scale

    if( p<gf )
    {
        ds = (p+1)*gs_step;
    }
    else if( p>(len-gf) )
    {
        ds = (len-p+1)*gs_step;
    }
    if( ds>1 )
    {
        ds = 1;
    }

    return ds;
}
