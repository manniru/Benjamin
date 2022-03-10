#ifndef KD_TOKEN_LIST_H
#define KD_TOKEN_LIST_H

#include "kd_lattice.h"
#include "kd_token2.h"

class KdTokenList
{
public:
    KdTokenList();

    void insert(KdToken2 *tok);

    KdToken2 *head = NULL;
    KdToken2 *tail = NULL;
    bool prune_forward_links = true;
    bool prune_tokens = true;
};

#endif // KD_TOKEN_LIST_H
