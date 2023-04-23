#ifndef MM_USAGE_H
#define MM_USAGE_H

#include <stdio.h>
#include <QVector>
#include <QObject>
#include <QTimer>
#include <Windows.h>

#include <TCHAR.h>
#include <pdh.h>

#define MOM_HISTORY_LEN 5

// You can also use L"\\Processor(*)\\% Processor Time"
// and get individual CPU values with PdhGetFormattedCounterArray()
// "\\Processor(_Total)\\% Processor Time" is actual usage and always
// less than "\Processor Information(_Total)\% Processor Utility"
// but "Processor Utility" is the same as task manager report
// and it could go to more than 100% if turbo mode


class MmUsage : public QObject
{
    Q_OBJECT
public:
    explicit MmUsage(QObject *parent = nullptr);
    ~MmUsage();

    QString getLabel();

private slots:
    void updateCpuUsage();

private:
    uint64_t FromFileTime( const FILETIME& ft );

    QTimer *timer;
    int     cpu_usage;
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;
    double cpu_history[MOM_HISTORY_LEN];
    int history_id;
    SYSTEM_INFO m_systemInfo;
};

#endif // MM_USAGE_H
