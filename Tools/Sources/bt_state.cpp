#include "bt_state.h"
#include <QDebug>
#include <QFile>
#include "ta_ini.h"

BtState::BtState()
{
    state = BT_NORM_MODE;

    if( QFile::exists(BT_CONFIG_PATH) )
    {
        qDebug() << "Load config file:" << BT_CONFIG_PATH;
        load_config();
    }
    else
    {
        qDebug() << "No config file, Load default values";
#ifdef WIN32
        system("copy Resources/BaTool.conf BaTool.conf");
#else //OR __linux
        system("cp Resources/BaTool.conf BaTool.conf");
#endif
        load_default();
    }
}

void BtState::load_config()
{
    TaINI *config = ini_load("BaTool.conf");
    fst_path       = ini_get(config, "model", "fst");;
    mdl_path       = ini_get(config, "model", "mdl");;
    cmvn_stat_path = ini_get(config, "model", "cmvn");;
//    qDebug() << "name:" << cmvn_stat_path;

    // decoder
    QString buf;
    buf = ini_get(config, "decoder", "max_active");
    max_active = buf.toInt();

    buf = ini_get(config, "decoder", "min_active");
    min_active = buf.toInt();

    buf = ini_get(config, "decoder", "train_max");
    train_max = buf.toInt();

    buf = ini_get(config, "decoder", "min_sil");
    min_sil = buf.toInt();

    // captain
    buf = ini_get(config, "captain", "hard_threshold");
    hard_threshold = buf.toDouble();
}

void BtState::load_default()
{
    fst_path       = BT_FST_PATH;
    mdl_path       = BT_OAMDL_PATH;
    cmvn_stat_path = BT_GCMVN_PATH;

    // decoder
    max_active = BT_MAX_ACTIVE;
    min_active = BT_MIN_ACTIVE;
    train_max  = BT_TRA_MAX;
    min_sil    = BT_MIN_SIL;

    // captain
    hard_threshold = KAL_HARD_TRESHOLD;
}
