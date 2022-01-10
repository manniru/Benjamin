#include "backend.h"
#include <QThread>

int getIntCommand(char *command)
{
    FILE *fp;
    int returnData;

    char path[1035];

    /* Open the command for reading. */
    fp = popen(command, "r");

    if (fp == NULL)
    {
        printf("Failed to run command\n" );
        return -1;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path)-1, fp) != NULL)
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

    if (fp == NULL) {
      printf("Failed to run command\n" );
      return returnData;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
      returnData = QString(path);
    }

    returnData.remove('\n');

    /* close */
    pclose(fp);
    return returnData;
}

QString getDiffTime(clock_t start)
{
    QString ret;
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    ret  = QString::number(qRound(cpu_time_used*1000));
    ret += "ms";
    return  ret;
}
