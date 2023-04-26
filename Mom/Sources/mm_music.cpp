#include "mm_music.h"
#include <QDebug>

MmMusic::MmMusic(QObject *parent) : QObject(parent)
{
    emul = new MmKeyEmulator;
}

MmMusic::~MmMusic()
{
    ;
}

QString MmMusic::getLabel()
{
    QString label;

    //prev
    label  = "%{A1:prev:}";
    label += "  \uf04a ";
    label += " %{A1} ";

    //play
    label += "%{A1:play:}";
    label += " \uf04b ";
    label += " %{A1} ";

    //next
    label += "%{A1:next:}";
    label += " \uf04e ";
    label += " %{A1}  ";

    return label;
}

void MmMusic::nextClick()
{
    qDebug() << "next";
    emul->sendKey(VK_MEDIA_NEXT_TRACK);
}

void MmMusic::playClick()
{
    qDebug() << "play";
    emul->sendKey(VK_MEDIA_PLAY_PAUSE);
}

void MmMusic::prevClick()
{
    qDebug() << "prev";
    emul->sendKey(VK_MEDIA_PREV_TRACK);
}


