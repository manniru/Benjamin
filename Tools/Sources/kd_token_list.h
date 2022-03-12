#ifndef KD_TOKEN_LIST_H
#define KD_TOKEN_LIST_H

#include "kd_lattice.h"
#include "kd_token.h"

class KdTokenList
{
public:
    KdTokenList();

    void insert(KdToken *tok);

    KdToken *head = NULL;
    KdToken *tail = NULL;
    bool prune_forward_links = true;
    bool prune_tokens = true;
};

#endif // KD_TOKEN_LIST_H
