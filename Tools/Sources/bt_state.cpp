#include "bt_state.h"

BtState::BtState(QObject *parent) : QObject(parent)
{

}

void BtState::setMode(int mode)
{
    i_mode = mode;
}

int BtState::getMode()
{
    return i_mode;
}

