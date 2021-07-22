#include "bt_state.h"

ReState::ReState(QObject *parent) : QObject(parent)
{

}

void ReState::setMode(int mode)
{
    i_mode = mode;
}

int ReState::getMode()
{
    return i_mode;
}

