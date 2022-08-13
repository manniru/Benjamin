#include "mm_monitor.h"
#include <QWindow>
#include <QDebug>

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
    MONITORINFOEXA mi;
    memset(&mi, 0, sizeof(mi));
    mi.cbSize = sizeof(mi);
    int success = GetMonitorInfoA(hMonitor, &mi);
    if( success==0 )
    {
        return TRUE;
    }

    int isPrimary = mi.dwFlags;
    qDebug() << "is primary ="
             << isPrimary
             << mi.szDevice;

    return TRUE;
}

MmMonitor::MmMonitor(QObject *root, QObject *parent) : QObject(parent)
{
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, NULL);
    QueryDisplay();
}

MmMonitor::~MmMonitor()
{
}

void MmMonitor::QueryDisplay()
{
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    UINT32 flags = QDC_ONLY_ACTIVE_PATHS;
    LONG isError = ERROR_INSUFFICIENT_BUFFER;

    UINT32 pathCount, modeCount;
    isError = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

    if( isError )
    {
        return;
    }

    // Allocate the path and mode arrays
    paths.resize(pathCount);
    modes.resize(modeCount);

    // Get all active paths and their modes
    isError = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

    // The function may have returned fewer paths/modes than estimated
    paths.resize(pathCount);
    modes.resize(modeCount);


    if ( isError )
    {
        return;
    }

    // For each active path
    int len = paths.size();
    for (int i=0 ; i<len ; i++ )
    {
        // Find the target (monitor) friendly name
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
        targetName.header.adapterId = paths[i].targetInfo.adapterId;
        targetName.header.id = paths[i].targetInfo.id;
        targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        targetName.header.size = sizeof(targetName);
        isError = DisplayConfigGetDeviceInfo(&targetName.header);

        if( isError )
        {
            return;
        }
        QString mon_name = "Unknown";
        if( targetName.flags.friendlyNameFromEdid )
        {
            mon_name = QString::fromStdWString(
                       targetName.monitorFriendlyDeviceName);
        }

        if( mon_name=="DELL U2415" )
        {
            state = MM_MON_DAVID;
        }
        else if( mon_name=="PHILIPS" )
        {
            state = MM_MON_UNKNOWN;
        }
        qDebug() << "Monitor " << mon_name;
    }
}

void MmMonitor::ListDisplay()
{
    ;
}

//BOOL AppBar_SetSide(HWND hwnd)
void MmMonitor::SetSide()
{

}
