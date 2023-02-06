#ifndef MM_SOUND_H
#define MM_SOUND_H

#include <stdio.h>
#include <QVector>
#include <Windows.h>
#include <mmdeviceapi.h>

#include "mm_virt.h"
#include "mm_lua.h"

class MmSound : public QObject
{
    Q_OBJECT
public:
    explicit MmSound(QObject *parent = nullptr);
    ~MmSound();

    void leftClick();
    QString getLabel();

private:
    QString getName(IMMDevice *dev);
    int     getVolume(IMMDevice *dev);
    int     isHeadset(IMMDevice *dev);
    void    setDevice(LPWSTR dev_iid);
    int     getNextIndex();

    QTimer *timer;
    IMMDeviceEnumerator *device_enum;
    IMMDeviceCollection *collection;
};

#endif // MM_SOUND_H
