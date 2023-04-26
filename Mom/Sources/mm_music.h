#ifndef MM_MUSIC_H
#define MM_MUSIC_H

#include <stdio.h>
#include <QVector>
#include <QObject>
#include <QTimer>
#include <Windows.h>
#include "mm_key_emulator.h"

class MmMusic : public QObject
{
    Q_OBJECT
public:
    explicit MmMusic(QObject *parent = nullptr);
    ~MmMusic();

    QString getLabel();
    void nextClick();
    void playClick();
    void prevClick();

private:
    MmKeyEmulator *emul;
};

#endif // MM_MUSIC_H
