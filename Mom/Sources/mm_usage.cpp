#include "mm_usage.h"
#include <QDebug>

MmUsage::MmUsage(QObject *parent) : QObject(parent)
{
//    HRESULT hr = ::CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
//                                    CLSCTX_INPROC_SERVER,
//                                    IID_PPV_ARGS(&device_enum));
//    if( hr )
//    {
//        qDebug() << "CoCreateInstance MMDeviceEnumerator failed"
//                 << hr;
//        return;
//    }
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    // You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
    PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time",
                         NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()),
            this, SLOT(updateCpuUsage()));

    cpu_usage  = 0;
    history_id = 0;
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

    cpu_history[history_id] = counterVal.doubleValue;
//    FILETIME a0, a1, a2;

//    GetSystemTimes(&a0, &a1, &a2);

//    uint64_t idl = FromFileTime( a0 );
//    uint64_t ker = FromFileTime( a1 );
//    uint64_t usr = FromFileTime( a2 );

//    uint64_t cpu = (ker + usr) * 100 / (ker + usr + idl);
//    cpu_history[history_id] = (ker + usr) * 100 / (ker + usr + idl);
//    qDebug()<< "cpu = " << cpu;

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
    cpu_usage = sum/MOM_HISTORY_LEN;
}

uint64_t MmUsage::FromFileTime( const FILETIME& ft )
{
    ULARGE_INTEGER uli = { 0 };
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}

