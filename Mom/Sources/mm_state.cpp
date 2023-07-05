#include "mm_state.h"
#include "ta_ini.h"
#include <QDebug>
#include <QFile>
#include <QDir>

MmState::MmState()
{
    QString path = "..";
    path += QDir::separator();
    path += BT_CONFIG_PATH;

    if( QFile::exists(path) )
    {
        qDebug() << "Load config file:" << path;
        load_config();
    }
    else
    {
        qDebug() << "No config file, Load default values";
        load_default();
    }
}

void MmState::load_config()
{
    TaINI *config = ini_load("BaTool.conf");
    mic_name = ini_get(config, "misc", "mic");
}

void MmState::load_default()
{
    mic_name       = MM_MIC_NAME;
}
