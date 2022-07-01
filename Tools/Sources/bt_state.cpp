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
        load_default();
    }
}

void BtState::load_config()
{
    TaINI *config = ini_load("BaTool.conf");
    char *name = ini_get(config, "Model", "fst");
    qDebug() << "name:" << name;
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
