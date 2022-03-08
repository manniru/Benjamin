#include "kd_decoder.h"
#include <QDebug>

using namespace kaldi;

KdDecoder::KdDecoder()
{
    num_elements = 0;
    elements.SetSize(3000);  // just so on the first frame we do something reasonable.
}

KdDecoder::~KdDecoder()
{
    DeleteElems(elements.Clear());
    ClearActiveTokens();
}

void KdDecoder::InitDecoding(KdDecodable *dcodable)
{
    // clean up from last time:
    DeleteElems(elements.Clear());
    cost_offsets.clear();
    ClearActiveTokens();
    warned_ = false;
    num_elements = 0;
    final_costs_.clear();
    KdStateId start_state = fst_->Start();
    KALDI_ASSERT(start_state != fst::kNoStateId);
    frame_toks.resize(1);
    KdToken2 *start_tok = new KdToken2(0.0, 0.0, NULL);
    frame_toks[0].toks = start_tok;
    elements.Insert(start_state, start_tok);
    num_elements++;
    ProcessNonemitting(config_.beam);
    decodable = dcodable;
}

// Locates a token in elements or inserts a new to frame_toks[frame]->toks
// 'changed' true if a token created or cost changed.
KdDecoder::Elem* KdDecoder::updateToken(
        KdStateId state, float tot_cost, bool *changed)
{
    int frame = frame_toks.size() - 1;
    Elem *e_found = elements.Insert(state, NULL);

    if( e_found->val==NULL )
    {
        KdTokenList *tok_list = &frame_toks[frame];
        const float extra_cost = 0.0;
        // tokens on the currently final frame have zero extra_cost
        // as any of them could end up on the winning path.
        KdToken2 *new_tok = new KdToken2 (tot_cost, extra_cost, tok_list->toks);
        // NULL: no forward links yet
        tok_list->toks = new_tok;
        num_elements++;
        e_found->val = new_tok;
        if (changed)
        {
            *changed = true;
        }
        qDebug() << "New Tok---->" << state << "f" << frame;
    }
    else// replace old token
    {
        KdToken2 *tok = e_found->val;
        if( tok->tot_cost > tot_cost )
        {
            tok->tot_cost = tot_cost;
            // we don't allocate a new token, the old stays linked in frame_toks
            // we only replace the tot_cost
            if (changed)
            {
                *changed = true;
            }
        }
        else
        {
            if (changed)
            {
                *changed = false;
            }
        }
        qDebug() << "Upd Tok---->" << state << "f" << frame;
    }
//    printActive();
    return e_found;
}

// Get Cutoff and Also Update adaptive_beam
float KdDecoder::GetCutoff(Elem *list_head,
                           Elem **best_elem)
{
    float best_weight = std::numeric_limits<float>::infinity();
    // positive == high cost == bad.
    size_t count = 0;
    tmp_array_.clear();
    for (Elem *e = list_head; e != NULL; e = e->tail, count++)
    {
        float w = e->val->tot_cost;
        tmp_array_.push_back(w);
        if (w < best_weight)
        {
            best_weight = w;
            if (best_elem)
            {
                *best_elem = e;
            }
        }
    }

    float beam_cutoff = best_weight + config_.beam;
    float min_active_cutoff = std::numeric_limits<float>::infinity();
    float max_active_cutoff = std::numeric_limits<float>::infinity();

    KALDI_VLOG(6) << "Number of tokens active on frame " << frame_num
                  << " is " << tmp_array_.size();

    if (tmp_array_.size() > static_cast<size_t>(config_.max_active))
    {
        std::nth_element(tmp_array_.begin(),
                         tmp_array_.begin() + config_.max_active,
                         tmp_array_.end());
        max_active_cutoff = tmp_array_[config_.max_active];
    }
    if (max_active_cutoff < beam_cutoff)
    {
        // max_active is tighter than beam.
        adaptive_beam = max_active_cutoff - best_weight + config_.beam_delta;
        return max_active_cutoff;
    }
    if (tmp_array_.size() > static_cast<size_t>(config_.min_active))
    {
        if (config_.min_active == 0)
        {
            min_active_cutoff = best_weight;
        }
        else
        {
            std::nth_element(tmp_array_.begin(),
                             tmp_array_.begin() + config_.min_active,
                             tmp_array_.size() > static_cast<size_t>(config_.max_active) ?
                                 tmp_array_.begin() + config_.max_active :
                                 tmp_array_.end());
            min_active_cutoff = tmp_array_[config_.min_active];
        }
    }
    if (min_active_cutoff > beam_cutoff)
    { // min_active is looser than beam.
        adaptive_beam = min_active_cutoff - best_weight + config_.beam_delta;
        return min_active_cutoff;
    }
    else
    {
        adaptive_beam = config_.beam;
        return beam_cutoff;
    }
}

double KdDecoder::GetBestCutoff(Elem *best_elem)
{
    double cutoff = KD_INFINITY;
    int frame = cost_offsets.size();

    cost_offsets.push_back(0.0);
    if( best_elem )
    {
        KdStateId state = best_elem->key;
        KdToken2 *tok = best_elem->val;
        float cost_offset = -tok->tot_cost;

        for( fst::ArcIterator<KdFST> aiter(*fst_, state) ; !aiter.Done() ; aiter.Next() )
        {
            const KdArc &arc = aiter.Value();
            if( arc.ilabel!=0 )
            {
                float arc_cost = -decodable->LogLikelihood(frame_num, arc.ilabel);
                float new_weight = arc.weight.Value() + (cost_offset + tok->tot_cost)
                                 + arc_cost + adaptive_beam;
                if( new_weight<cutoff )
                {
                    cutoff = new_weight;
                }
            }
        }
        cost_offsets[frame] = cost_offset;
    }

    return cutoff;
}

// Processes for one frame.
float KdDecoder::ProcessEmitting()
{
    frame_toks.push_back(KdTokenList()); //add new frame tok

    Elem *final_toks = elements.Clear();
    Elem *best_elem = NULL;

    float cutoff = GetCutoff(final_toks, &best_elem);
    float next_cutoff = GetBestCutoff(best_elem);
    qDebug() << "best_state" << best_elem->key
             << "cutoff" << cutoff << next_cutoff
             << "uframe" << frame_num;
    QString dbg_buf;
    int count = 0;
    Elem *e_tail;
    for( Elem *e=final_toks ; e!=NULL ; e=e_tail )
    {
        dbg_buf += QString::number(e->key);
        dbg_buf += "->";
        count++;
        if( e->val->tot_cost<=cutoff )
        {
            next_cutoff = PEmittingElem(e, next_cutoff);
        }
        e_tail = e->tail;
        elements.Delete(e);
    }
    qDebug() << "ProcessEmitting---->" << count;
    qDebug() << dbg_buf;

    frame_num++;
    return next_cutoff;
}

// Processes for one frame.
void KdDecoder::ProcessNonemitting(float cutoff)
{
    // need for reverse
    int count = 0;
    QString dbg_buf;
    for (Elem *e = const_cast<Elem *>(elements.GetList()); e != NULL;  e = e->tail)
    {
        KdStateId state = e->key;
        dbg_buf += QString::number(state);
        dbg_buf += "->";
//        qDebug() << "NNS---->" << state;
        count++;
        if (fst_->NumInputEpsilons(state) != 0)
        {
            PNonemittingElem(e, cutoff);
        }
    }

    qDebug() << "NN---->" << count
             << "u" << frame_num;
    qDebug() << dbg_buf;
}

// Processes Single Emiting Elem
float KdDecoder::PEmittingElem(Elem *e, float next_cutoff)
{
    int frame = frame_toks.size() - 2;

    KdStateId e_state = e->key;
    KdToken2 *e_tok = e->val;

    for(fst::ArcIterator<KdFST> aiter(*fst_, e_state); !aiter.Done(); aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel!=0 )
        {
            float new_weight = decodable->LogLikelihood(frame_num, arc.ilabel);
            float ac_cost = cost_offsets[frame] - new_weight;
            float graph_cost = arc.weight.Value();
            float tot_cost = e_tok->tot_cost + ac_cost + graph_cost;

            if( tot_cost>=next_cutoff )
            {
                continue;
            }
            else if( tot_cost+adaptive_beam<next_cutoff )
            {
                next_cutoff = tot_cost + adaptive_beam; // prune by best current token
            }

            Elem *e_found = updateToken(arc.nextstate, tot_cost,
                                          NULL);
            KdToken2 *ef_tok = e_found->val;

            ef_tok->ilabel = arc.ilabel;
            ef_tok->olabel = arc.olabel;
            ef_tok->graph_cost = graph_cost;
            ef_tok->acoustic_cost = ac_cost;
//            qDebug() << "PIEE---->" << e_state
//                     << "to" << ac_cost;
            // Add ForwardLink from tok to next_tok (put on head of list tok->links)
            e_tok->links = new KdFLink(ef_tok, arc.ilabel, arc.olabel,
                                       graph_cost , ac_cost, e_tok->links);
        }
    }
    return next_cutoff;
}

// Processes Single Non Emiting Elem
void KdDecoder::PNonemittingElem(Elem *e, float cutoff)
{
    KdStateId e_state = e->key;
    KdToken2 *e_tok = e->val;
    float cur_cost = e_tok->tot_cost;
    if( cur_cost>=cutoff )
    {
        return;// Don't bother processing
    }
    // If "tok" has any existing forward links, delete them,
    // because we're about to regenerate them.  This is a kind
    // of non-optimality (since most states are emitting it's not a huge issue.)
    DeleteForwardLinks(e_tok); // necessary when re-visiting

    for( fst::ArcIterator<KdFST> aiter(*fst_, e_state) ; !aiter.Done() ; aiter.Next() )
    {
        const KdArc &arc = aiter.Value();
        if( arc.ilabel==0 ) // nonemitting
        {
            float graph_cost = arc.weight.Value();
            float tot_cost = cur_cost + graph_cost;
            if( tot_cost<cutoff )
            {
                bool changed;
                Elem *e_found = updateToken(arc.nextstate, tot_cost,
                                             &changed);
                KdToken2 *ef_tok = e_found->val;

                ef_tok->ilabel = 0;
                ef_tok->olabel = arc.olabel;
                ef_tok->graph_cost = graph_cost;
                ef_tok->acoustic_cost = 0;
//                qDebug() << "PNEE---->" << e_state
//                         << "to" << graph_cost;

                e_tok->links = new KdFLink(ef_tok, 0, arc.olabel,
                                         graph_cost, 0, e_tok->links);

                if (changed && fst_->NumInputEpsilons(arc.nextstate) != 0)
                {
                    PNonemittingElem(e_found, cutoff);
                }
            }
        }
    }
}

// Deletes the elements of the singly linked list tok->links.
void KdDecoder::DeleteForwardLinks(KdToken2 *tok)
{
    KdFLink *l = tok->links, *m;
    while( l!=NULL )
    {
        m = l->next;
        delete l;
        l = m;
    }
    tok->links = NULL;
}

// the elements indexed by state. The function DeleteElems returns
// ownership of elements structure for reuse, The KdToken pointers
// are reference-counted and are ultimately deleted in PruneTokensForFrame,
void KdDecoder::DeleteElems(Elem *list)
{
    for (Elem *e = list, *e_tail; e != NULL; e = e_tail)
    {
        e_tail = e->tail;
        elements.Delete(e);
    }
}

void KdDecoder::ClearActiveTokens()
{
    for (size_t i = 0; i < frame_toks.size(); i++)
    {
        KdToken2 *tok=frame_toks[i].toks;
        while( tok!=NULL )
        {
            DeleteForwardLinks(tok);
            KdToken2 *next_tok = tok->next;
            delete tok;
            num_elements--;
            tok = next_tok;
        }
    }
    frame_toks.clear();
    KALDI_ASSERT(num_elements == 0);
}

void KdDecoder::printActive()
{
    int count = 0;
    QString dbg_buf;
    for (Elem *e = const_cast<Elem *>(elements.GetList()); e != NULL;  e = e->tail)
    {
        KdStateId state = e->key;
        dbg_buf += QString::number(state);
        dbg_buf += "->";
        count++;
    }

    qDebug() << "<---- STATE" << count
             << "---->";
    qDebug() << dbg_buf;
    qDebug() << "-------------";
}

// outputs a list in topological order
void KdDecoder::TopSortTokens(KdToken2 *tok_list,
                              std::vector<KdToken2 *> *out)
{
    unordered_map<KdToken2*, int> token2pos;
    typedef typename unordered_map<KdToken2*, int>::iterator IterType;
    int num_toks = 0;
    for (KdToken2 *tok = tok_list; tok != NULL; tok = tok->next)
        num_toks++;
    int cur_pos = 0;
    // We assign the tokens numbers num_toks - 1, ... , 2, 1, 0.
    // This is likely to be in closer to topological order than
    // if we had given them ascending order, because of the way
    // new tokens are put at the front of the list.
    for (KdToken2 *tok = tok_list; tok != NULL; tok = tok->next)
    {
        token2pos[tok] = num_toks - ++cur_pos;
    }

    unordered_set<KdToken2*> reprocess;

    for( IterType iter=token2pos.begin() ; iter!=token2pos.end() ; ++iter )
    {
        KdToken2 *tok = iter->first;
        int pos = iter->second;
        for (KdFLink *link = tok->links; link != NULL; link = link->next)
        {
            if (link->ilabel == 0)
            {
                // We only need to consider epsilon links, since non-epsilon links
                // transition between frames and this function only needs to sort a list
                // of tokens from a single frame.
                IterType following_iter = token2pos.find(link->next_tok);
                if (following_iter != token2pos.end())
                { // another token on this frame,
                    // so must consider it.
                    int next_pos = following_iter->second;
                    if (next_pos < pos)
                    { // reassign the position of the next KdToken.
                        following_iter->second = cur_pos++;
                        reprocess.insert(link->next_tok);
                    }
                }
            }
        }
        // In case we had previously assigned this token to be reprocessed, we can
        // erase it from that set because it's "happy now" (we just processed it).
        reprocess.erase(tok);
    }

    size_t max_loop = 1000000;
    size_t loop_count; // max_loop is to detect epsilon cycles.
    for( loop_count=0 ; !reprocess.empty() && loop_count<max_loop; ++loop_count )
    {
        std::vector<KdToken2*> reprocess_vec;
        for (typename unordered_set<KdToken2*>::iterator iter = reprocess.begin();
             iter != reprocess.end(); ++iter)
            reprocess_vec.push_back(*iter);
        reprocess.clear();
        for (typename std::vector<KdToken2*>::iterator iter = reprocess_vec.begin();
             iter != reprocess_vec.end(); ++iter) {
            KdToken2 *tok = *iter;
            int pos = token2pos[tok];
            // Repeat the processing we did above (for comments, see above).
            for (KdFLink *link = tok->links; link != NULL; link = link->next)
            {
                if (link->ilabel == 0)
                {
                    IterType following_iter = token2pos.find(link->next_tok);
                    if (following_iter != token2pos.end())
                    {
                        int next_pos = following_iter->second;
                        if (next_pos < pos)
                        {
                            following_iter->second = cur_pos++;
                            reprocess.insert(link->next_tok);
                        }
                    }
                }
            }
        }
    }

    out->clear();
    out->resize(cur_pos, NULL);
    for (IterType iter = token2pos.begin(); iter != token2pos.end(); ++iter)
    {
        (*out)[iter->second] = iter->first;
    }
}
