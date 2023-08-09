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
//    adjustSetting();
}

MmMonitor::~MmMonitor()
{
}

int MmMonitor::monitorNum()
{
    return mon_names.size();
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
    isError = QueryDisplayConfig(flags, &pathCount, paths.data(),
                                 &modeCount, modes.data(), nullptr);

    // The function may have returned fewer paths/modes than estimated
    paths.resize(pathCount);
    modes.resize(modeCount);


    if( isError )
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

        // Find the adapter device name
        DISPLAYCONFIG_ADAPTER_NAME adapterName = {};
        adapterName.header.adapterId = paths[i].targetInfo.adapterId;
        adapterName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
        adapterName.header.size = sizeof(adapterName);

        isError = DisplayConfigGetDeviceInfo(&adapterName.header);
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
        qDebug() << "Monitor " << mon_name
//                 << QString::fromStdWString(adapterName.adapterDevicePath)
                 << paths[i].sourceInfo.id;

        mon_names << mon_name;
    }
}

void MmMonitor::adjustSetting()
{
    DEVMODE devmode = { 0 };

    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);
    int deviceIndex = 0;
    while( EnumDisplayDevices(0, deviceIndex, &dd, 0) )
    {
        std::wstring deviceName = dd.DeviceName;
        int monitorIndex = 0;
        while( EnumDisplayDevices(deviceName.c_str(), monitorIndex, &dd, 0) )
        {
            EnumDisplaySettings(deviceName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);
            qDebug() << deviceIndex
                     << QString::fromStdWString(deviceName)
                     << ", " << QString::fromStdWString(dd.DeviceString)
                     << dd.StateFlags << " X "
                     << devmode.dmPelsWidth << " X "
                     << devmode.dmPelsHeight;
            monitorIndex++;
        }
        deviceIndex++;
    }

//    EnumDisplaySettings( NULL,ENUM_CURRENT_SETTINGS, &devmode );

//    devmode.dmSize = sizeof(DEVMODE);
//    devmode.dmPelsWidth = 1920; //take last item maximum value
//    devmode.dmPelsHeight = 1080; //take last item maximum value

//    long result = ChangeDisplaySettings(&devmode, DM_PELSWIDTH | DM_PELSHEIGHT);
}
