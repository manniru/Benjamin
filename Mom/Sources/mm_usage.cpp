#include "mm_usage.h"
#include <QDebug>

MmUsage::MmUsage(QObject *parent) : QObject(parent)
{
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddCounter(cpuQuery, L"\\Processor Information(_Total)\\% Processor Utility",
                         NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(updateCpuUsage()));

    cpu_usage  = 0;
    history_id = 0;
    for( int i=0 ; i<MOM_HISTORY_LEN ; i++ )
    {
        cpu_history[i] = 0;
    }
    timer->start(100);
}

MmUsage::~MmUsage()
{
    ;
}

QString MmUsage::getLabel()
{
    QString label;

    label  = "%{U#E7C678}%{+U}";

    label += "  \uf1b2 ";

    label += QString::number(cpu_usage);

    label += "% %{-U}  ";

    return label;
}

void MmUsage::updateCpuUsage()
{
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                NULL, &counterVal);

    cpu_history[history_id] = qRound(counterVal.doubleValue);

    history_id++;
    if( history_id>=MOM_HISTORY_LEN )
    {
        history_id = 0;
    }

    double sum = 0;
    for( int i=0; i<MOM_HISTORY_LEN; i++ )
    {
        sum += cpu_history[i];
    }
    cpu_usage = qRound(sum/MOM_HISTORY_LEN);
}

uint64_t MmUsage::FromFileTime( const FILETIME& ft )
{
    ULARGE_INTEGER uli = { 0 };
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

