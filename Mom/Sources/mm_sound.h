#ifndef MM_SOUND_H
#define MM_SOUND_H

#include <mmdeviceapi.h>
#include <stdio.h>
#include <Windows.h>
#include <QVector>
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
    void    setDevice(QString name);

    QTimer *timer;
    IMMDeviceEnumerator *device_enum;
};

#endif // MM_SOUND_H
