#ifndef ABCONSOLEREADER_H
#define ABCONSOLEREADER_H

#include <QObject>
#include <windows.h>

#define CONSOLE_BUF_SIZE 4096

class AbConsoleReader : public QObject
{
    Q_OBJECT
public:
    explicit AbConsoleReader(QObject *parent = nullptr);
    ~AbConsoleReader();

    HANDLE handle = NULL;

public slots:
    void run();

signals:
    void readyData(QString data);
    void readyCommand();

private:
    void processLine(QString line);
};

#endif // ABCONSOLEREADER_H
