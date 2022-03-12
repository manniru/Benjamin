#include "kd_token_list.h"

using namespace kaldi;
KdTokenList::KdTokenList()
{
    head = NULL;
    tail = NULL;
}

void KdTokenList::insert(KdToken *tok)
{
    if( head==NULL )
    {
        head = tok;
        tail = tok;
    }
    else
    {
        tail->next = tok;
        tok->prev  = tail;
        tail = tok;
    }
}
