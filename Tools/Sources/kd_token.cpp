#include "kd_token.h"
#include <QDebug>

int global_id = 0;

KdToken::KdToken(float total_cost)
{
    cost = total_cost;
    prune_cost = 0;
    links = NULL;
    next = NULL;
    prev = NULL;

    state = KD_INVALID_STATE;
    m_state = KD_INVALID_STATE;
    tok_id = global_id;
    global_id++;
}
