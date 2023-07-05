#ifndef MM_SOUND_H
#define MM_SOUND_H

#include <stdio.h>
#include <QVector>
#include <Windows.h>
#include <mmdeviceapi.h>

#include "mm_virt.h"
#include "mm_lua.h"
#include "mm_state.h"

class MmSound : public QObject
{
    Q_OBJECT
public:
    explicit MmSound(MmState *st, QObject *parent = nullptr);
    ~MmSound();

    void leftClick();
    void volumeUp();
    void volumeDown();
    QString getLabel();

private slots:
    void micTimeOut();

private:
    QString getName(IMMDevice *dev);
    int     getVolume(IMMDevice *dev);
    int     isHeadset(IMMDevice *dev);
    void    setDevice(LPWSTR dev_iid);
    void    setVolume(IMMDevice *spkr_dev, int volume);
    void    updateDefualtMic();
    int     getNextIndex();
    void    updateMic();

    MmState   *state;
    QTimer    *mic_timer;
    IMMDevice *mic_dev;
    IMMDeviceEnumerator *device_enum;
    IMMDeviceCollection *collection;
};

#endif // MM_SOUND_H
