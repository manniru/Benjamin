#ifndef MMSTATE_H
#define MMSTATE_H

#include <QString>
#define BT_CONFIG_PATH    "BaTool.conf"
#define MM_MIC_NAME       ""

class MmState
{
public:
    MmState();

    QString mic_name;

private:
    void load_config();
    void load_default();
};

#endif // MMSTATE_H
