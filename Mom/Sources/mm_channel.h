#ifndef MM_CHANNEL_H
#define MM_CHANNEL_H

#include <QObject>
#include <QThread>
#include <windows.h>
#include "mm_config.h"

// The buffer size specified should be small enough that your
// process will not run out of nonpaged pool, but large enough
// to accommodate typical requests.
#define BUFFER_SIZE (1024 * 8)

class MmChannel : public QObject
{
    Q_OBJECT
public:
    MmChannel(QObject *parent = NULL);
    ~MmChannel();

signals:
    void set_virt(int val);
    void meta(QString val);

public slots:
    void listenPipe();

private:
    void createPipe();
    void processCommand(QString cmd, QString arg);
    void processLine(QString line);

    HANDLE hPipe;
};



#endif // MM_CHANNEL_H
