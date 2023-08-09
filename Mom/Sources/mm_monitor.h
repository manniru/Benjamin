#ifndef MM_MONITOR_H
#define MM_MONITOR_H

#include <QObject>
#include <QString>
#include <QVector>

#include <Windows.h>

#define MM_MON_UNKNOWN 0
#define MM_MON_DAVID   1
#define MM_MON_OPTICS  2

class MmMonitor : public QObject
{
    Q_OBJECT
public:
    explicit MmMonitor(QObject *root, QObject *parent = nullptr);
    ~MmMonitor();

    int monitorNum();

private:
    void adjustSetting();
    void QueryDisplay();

    int state;
    QVector<QString> mon_names;
};

#endif // MM_MONITOR_H
