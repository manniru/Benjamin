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
    fst_path       = ini_get(config, "model", "fst");
    mdl_path       = ini_get(config, "model", "mdl");
    word_path      = ini_get(config, "model", "word");
    cmvn_stat_path = ini_get(config, "model", "cmvn");
//    qDebug() << "name:" << cmvn_stat_path;

    // decoder
    QString buf;
    buf = ini_get(config, "decoder", "max_active");
    max_active = buf.toInt();

    buf = ini_get(config, "decoder", "min_active");
    min_active = buf.toInt();

    buf = ini_get(config, "decoder", "train_max");
    train_max = buf.toInt();

    buf = ini_get(config, "decoder", "ac_scale");
    ac_scale = buf.toDouble();

    buf = ini_get(config, "decoder", "min_sil");
    min_sil = buf.toInt();

    // captain
    buf = ini_get(config, "captain", "hard_threshold");
    hard_threshold = buf.toDouble();

    // misc
    mic_name = ini_get(config, "misc", "mic");
    channel_np = ini_get(config, "misc", "channel");
    if( channel_np.length()==0 )
    {
        channel_np = BT_PIPE_ADDRESS;
    }
}

void BtState::load_default()
{
    // model
    fst_path       = BT_FST_PATH;
    mdl_path       = BT_OAMDL_PATH;
    word_path      = BT_WORDS_PATH;
    cmvn_stat_path = BT_GCMVN_PATH;

    // decoder
    max_active = BT_MAX_ACTIVE;
    min_active = BT_MIN_ACTIVE;
    train_max  = BT_TRA_MAX;
    ac_scale   = KAL_AC_SCALE;
    min_sil    = BT_MIN_SIL;

    // captain
    hard_threshold = KAL_HARD_TRESHOLD;

    // misc
    mic_name       = BT_MIC_NAME;
    channel_np     = BT_PIPE_ADDRESS;
}
