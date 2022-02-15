#include "kd_lattice_decoder.h"
#include <QDebug>

using namespace kaldi;

KdLatticeDecoder::KdLatticeDecoder()
{
    num_toks_ = 0;
    toks_.SetSize(1000);  // just so on the first frame we do something reasonable.
}

KdLatticeDecoder::~KdLatticeDecoder()
{
    DeleteElems(toks_.Clear());
    ClearActiveTokens();
}

void KdLatticeDecoder::InitDecoding(KdOnline2Decodable *dcodable)
{
    // clean up from last time:
    DeleteElems(toks_.Clear());
    cost_offsets.clear();
    ClearActiveTokens();
    warned_ = false;
    num_toks_ = 0;
    decoding_finalized_ = false;
    final_costs_.clear();
    KdStateId start_state = fst_->Start();
    KALDI_ASSERT(start_state != fst::kNoStateId);
    frame_toks.resize(1);
    KdToken2 *start_tok = new KdToken2(0.0, 0.0, NULL);
    frame_toks[0].toks = start_tok;
    toks_.Insert(start_state, start_tok);
    num_toks_++;
    ProcessNonemitting(config_.beam);
    decodable = dcodable;
}

// Locates a token in toks_ or inserts a new to frame_toks[frame]->toks
// 'changed' true if a token created or cost changed.
KdLatticeDecoder::Elem* KdLatticeDecoder::FindOrAddToken(
        KdStateId state, float tot_cost, bool *changed)
{
    int frame = frame_toks.size() - 1;
    Elem *e_found = toks_.Insert(state, NULL);
    if( e_found->val==NULL )
    {
        KdTokenList *tok_list = &frame_toks[frame];
        const float extra_cost = 0.0;
        // tokens on the currently final frame have zero extra_cost
        // as any of them could end up on the winning path.
        KdToken2 *new_tok = new KdToken2 (tot_cost, extra_cost, tok_list->toks);
        // NULL: no forward links yet
        tok_list->toks = new_tok;
        num_toks_++;
        e_found->val = new_tok;
        if (changed)
        {
            *changed = true;
        }
        return e_found;
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
        return e_found;
    }
}

// remove all links in frame_toks[frame].toks
// based on link_extra_cost>config_.lattice_beam
bool KdLatticeDecoder::PruneForwardLinks(
        int frame, bool *extra_costs_changed, float delta)
{
    *extra_costs_changed = false;
    bool links_pruned = false;
    KALDI_ASSERT(frame>=0 && frame<frame_toks.size());
    KdToken2 *start_tok = frame_toks[frame].toks;

    if( start_tok==NULL )
    {
        qDebug() << "PruneForwardLinks: No tokens alive @ " << frame;
    }

    bool changed = true;  // difference new minus old extra cost >= delta ?
    while( changed )
    {
        changed = false;
        for( KdToken2 *tok=start_tok ; tok!=NULL; tok=tok->next )
        {
            KdFLink *link, *prev_link = NULL;
            float tok_extra_cost = std::numeric_limits<float>::infinity();
            // tok_extra_cost is the best (min) of link_extra_cost of outgoing links
            for( link=tok->links ; link!=NULL; )
            {
                KdToken2 *next_tok = link->next_tok;
                float link_extra_cost = next_tok->extra_cost +
                        ((tok->tot_cost + link->acoustic_cost + link->graph_cost)
                         - next_tok->tot_cost);  // difference in brackets is >= 0
                // link_exta_cost is the difference in score between the best paths
                // through link source state and through link destination state
                KALDI_ASSERT(link_extra_cost == link_extra_cost);  // check for NaN
                if( link_extra_cost>config_.lattice_beam )  // delete link
                {
                    KdFLink *next_link = link->next;
                    if( prev_link!=NULL )
                    {
                        prev_link->next = next_link;
//                        prev_link->next_tok->link_tok = next_link->next_tok;
                    }
                    else
                    {
                        tok->links = next_link;
//                        tok->link_tok = next_link->next_tok;
                    }
                    delete link;
                    link = next_link;  // advance link but leave prev_link the same.
                    links_pruned = true;
                }
                else // update the tok_extra_cost if needed.
                {
                    if (link_extra_cost < 0.0)
                    {
                        if (link_extra_cost < -0.01)
                            KALDI_WARN << "Negative extra_cost: " << link_extra_cost;
                        link_extra_cost = 0.0;
                    }
                    if (link_extra_cost < tok_extra_cost)
                        tok_extra_cost = link_extra_cost;
                    prev_link = link;  // move to next link
                    link = link->next;
                }
            }
            if( fabs(tok_extra_cost-tok->extra_cost)>delta )
                changed = true;   // difference new minus old is bigger than delta
            if( tok_extra_cost!=std::numeric_limits<float>::infinity() )
            {
                tok->extra_cost = tok_extra_cost;
            }
            else
            {
                changed = false;
            }
            // will be +infinity or <= lattice_beam_.
            // infinity indicates, that no forward link survived pruning
        }
        if (changed)
        {
            *extra_costs_changed = true;
        }
    }

    return links_pruned;
}

float KdLatticeDecoder::FinalRelativeCost()
{
    if( !decoding_finalized_ )
    {
        float relative_cost;
        ComputeFinalCosts(NULL, &relative_cost, NULL);
        return relative_cost;
    }
    else
    {   // not allowed if FinalizeDecoding() has been called.
        return final_relative_cost_;
    }
}

// Prune away any tokens on this frame that have no forward links.
void KdLatticeDecoder::PruneTokensForFrame(int frame)
{
    KALDI_ASSERT( frame>=0 && frame<frame_toks.size() );

    KdToken2 *&start_token = frame_toks[frame].toks;
    if( start_token==NULL )
    {
        qDebug() << "PruneTokensForFrame: No tokens alive @" << frame;
    }

    KdToken2 *tok, *next_tok, *prev_tok = NULL;
    for( tok=start_token ; tok!=NULL ; tok=next_tok )
    {
        next_tok = tok->next;
        if (tok->extra_cost == std::numeric_limits<float>::infinity())
        {   // token is unreachable from end of graph
            if (prev_tok != NULL)
            {
                prev_tok->next = tok->next;
            }
            else
            {
                start_token = tok->next;
            }
            delete tok;
            num_toks_--;
            qDebug() << "delete" << frame << "num_toks_" << num_toks_;
        }
        else
        {  // fetch next KdToken
            prev_tok = tok;
        }
    }
}

// Go backwards through still-alive tokens, pruning them if the
// forward+backward cost is more than lat_beam away from the best path.  It's
// possible to prove that this is "correct" in the sense that we won't lose
// anything outside of lat_beam, regardless of what happens in the future.
// delta controls when it considers a cost to have changed enough to continue
// going backward and propagating the change.  larger delta -> will recurse
// less far.
void KdLatticeDecoder::PruneActiveTokens(float delta)
{
    int cur_frame_plus_one = frame_toks.size();
    int num_toks_begin = num_toks_;

    for(int f=cur_frame_plus_one-1 ; f>=0 ; f-- )
    {
        // Reason why we need to prune forward links in this situation:
        // (1) we have never pruned them (new TokenList)
        // (2) we have not yet pruned the forward links to the next f,
        // after any of those tokens have changed their extra_cost.
        if( frame_toks[f].prune_forward_links )
        {
            bool extra_costs_changed = false;
            bool links_pruned;
            links_pruned = PruneForwardLinks(f, &extra_costs_changed, delta);

            if( extra_costs_changed && f>0 ) // any token has changed extra_cost
            {
                frame_toks[f-1].prune_forward_links = true;
            }
            if (links_pruned) // any link was pruned
            {
                frame_toks[f].prune_tokens = true;
            }
            frame_toks[f].prune_forward_links = false; // job done
        }

        if( f+1<cur_frame_plus_one && frame_toks[f+1].prune_tokens )
        {
            PruneTokensForFrame(f+1);
            frame_toks[f+1].prune_tokens = false;
        }
    }
    KALDI_VLOG(4) << "PruneActiveTokens: pruned tokens from " << num_toks_begin
                  << " to " << num_toks_;
}

// computes final-costs for the final frame. It outputs to final-costs, a map from the KdToken*
// pointer to the final-prob of the corresponding state, for all Tokens
// that correspond to states that have final-probs.
void KdLatticeDecoder::ComputeFinalCosts(unordered_map<KdToken2 *, float> *final_costs,
        float *final_relative_cost, float *final_best_cost)
{
    KALDI_ASSERT(!decoding_finalized_);
    if (final_costs != NULL)
        final_costs->clear();
    const Elem *final_toks = toks_.GetList();
    float infinity = std::numeric_limits<float>::infinity();
    float best_cost = infinity,
            best_cost_with_final = infinity;

    while (final_toks != NULL) {
        KdStateId state = final_toks->key;
        KdToken2 *tok = final_toks->val;
        const Elem *next = final_toks->tail;
        float final_cost = fst_->Final(state).Value();
        float cost = tok->tot_cost,
                cost_with_final = cost + final_cost;
        best_cost = std::min(cost, best_cost);
        best_cost_with_final = std::min(cost_with_final, best_cost_with_final);
        if (final_costs != NULL && final_cost != infinity)
            (*final_costs)[tok] = final_cost;
        final_toks = next;
    }
    if (final_relative_cost != NULL)
    {
        if (best_cost == infinity && best_cost_with_final == infinity)
        {
            // Likely this will only happen if there are no tokens surviving.
            // This seems the least bad way to handle it.
            *final_relative_cost = infinity;
        }
        else
        {
            *final_relative_cost = best_cost_with_final - best_cost;
        }
    }
    if (final_best_cost != NULL)
    {
        if (best_cost_with_final != infinity)
        { // final-state exists.
            *final_best_cost = best_cost_with_final;
        } else
        { // no final-state exists.
            *final_best_cost = best_cost;
        }
    }
}

void KdLatticeDecoder::AdvanceDecoding()
{
    KALDI_ASSERT(!frame_toks.empty() && !decoding_finalized_ &&
                 "You must call InitDecoding() before AdvanceDecoding");
    int max_frame = decodable->NumFramesReady();

    KALDI_ASSERT( max_frame>=frame_num );

    while( frame_num<max_frame )
    {
        if( (frame_num%config_.prune_interval)==0 )
        {
            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float cost_cutoff = ProcessEmitting();
        ProcessNonemitting(cost_cutoff);
    }
}

// Get Cutoff and Also Update adaptive_beam
float KdLatticeDecoder::GetCutoff(Elem *list_head, size_t *tok_count,
                                  Elem **best_elem)
{
    float best_weight = std::numeric_limits<float>::infinity();
    // positive == high cost == bad.
    size_t count = 0;
    if (config_.max_active == std::numeric_limits<int>::max() &&
            config_.min_active == 0)
    {
        for (Elem *e = list_head; e != NULL; e = e->tail, count++)
        {
            float w = static_cast<float>(e->val->tot_cost);
            if (w < best_weight)
            {
                best_weight = w;
                if (best_elem) *best_elem = e;
            }
        }
        if (tok_count != NULL)
        {
            *tok_count = count;
        }
        adaptive_beam = config_.beam;
        return best_weight + config_.beam;
    }
    else
    {
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
        if (tok_count != NULL) *tok_count = count;

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
}

double KdLatticeDecoder::GetBestCutoff(Elem *best_elem)
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
float KdLatticeDecoder::ProcessEmitting()
{
    frame_toks.push_back(KdTokenList()); //add new frame tok

    Elem *final_toks = toks_.Clear();
    Elem *best_elem = NULL;
    size_t tok_cnt;

    float cutoff = GetCutoff(final_toks, &tok_cnt, &best_elem);
    float next_cutoff = GetBestCutoff(best_elem);

    Elem *e_tail;
    int id_t = 0;
    for( Elem *e=final_toks ; e!=NULL ; e=e_tail )
    {
        if( e->val->tot_cost<=cutoff )
        {
            next_cutoff = PEmittingElem(e, next_cutoff);
        }
        e_tail = e->tail;
        toks_.Delete(e);
//        qDebug() << "ProcessEmitting ID_T" << id_t;
        id_t++;
    }

    frame_num++;
    return next_cutoff;
}

// Processes for one frame.
void KdLatticeDecoder::ProcessNonemitting(float cutoff)
{
    KALDI_ASSERT(!frame_toks.empty());
    if( toks_.GetList()==NULL )
    {
        int frame = frame_toks.size() - 2;
        qDebug() << "Error, no surviving tokens: frame is " << frame;
        return;
    }

    // need for reverse
    queue_.clear();
    for (Elem *e = const_cast<Elem *>(toks_.GetList()); e != NULL;  e = e->tail)
    {
        KdStateId state = e->key;
        if (fst_->NumInputEpsilons(state) != 0)
        {
            queue_.push_back(e);
        }
    }

    int q_len = queue_.size();
    for( int i=0 ; i<q_len ; i++ )
    {
        PNonemittingElem(queue_[q_len-i-1], cutoff);
    }
}

// Processes Single Emiting Elem
float KdLatticeDecoder::PEmittingElem(Elem *e, float next_cutoff)
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

            Elem *e_found = FindOrAddToken(arc.nextstate, tot_cost,
                                          NULL);
            KdToken2 *ef_tok = e_found->val;

            ef_tok->ilabel = arc.ilabel;
            ef_tok->olabel = arc.olabel;
            ef_tok->graph_cost = graph_cost;
            ef_tok->acoustic_cost = ac_cost;
            ef_tok->link_tok = e_tok->link_tok;
            e_tok->link_tok = ef_tok;
            e_tok->is_linked = 1;
            // Add ForwardLink from tok to next_tok (put on head of list tok->links)
            e_tok->links = new KdFLink(ef_tok, arc.ilabel, arc.olabel,
                                       graph_cost , ac_cost, e_tok->links);

//            qDebug() << "h";
        }
    }
    return next_cutoff;
}


// Processes Single Non Emiting Elem
void KdLatticeDecoder::PNonemittingElem(Elem *e, float cutoff)
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
                Elem *e_found = FindOrAddToken(arc.nextstate, tot_cost,
                                             &changed);
                KdToken2 *ef_tok = e_found->val;

                ef_tok->ilabel = 0;
                ef_tok->olabel = arc.olabel;
                ef_tok->graph_cost = graph_cost;
                ef_tok->acoustic_cost = 0;
                ef_tok->link_tok = e_tok->link_tok;
                e_tok->link_tok = ef_tok;
                e_tok->is_linked = 1;

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
void KdLatticeDecoder::DeleteForwardLinks(KdToken2 *tok)
{
    KdFLink *l = tok->links, *m;
    while( l!=NULL )
    {
        m = l->next;
        delete l;
        l = m;
    }
    KdToken2 *tok_l = tok;
    while( tok_l!=NULL )
    {
        KdToken2 *m2 = tok_l->link_tok;
        tok_l->link_tok = NULL;
        tok_l = m2;
    }
    tok->links = NULL;
}

// the toks_ indexed by state. The function DeleteElems returns
// ownership of toks_ structure for reuse, The KdToken pointers
// are reference-counted and are ultimately deleted in PruneTokensForFrame,
void KdLatticeDecoder::DeleteElems(Elem *list)
{
    for (Elem *e = list, *e_tail; e != NULL; e = e_tail)
    {
        e_tail = e->tail;
        toks_.Delete(e);
    }
}

void KdLatticeDecoder::ClearActiveTokens()
{
    for (size_t i = 0; i < frame_toks.size(); i++)
    {
        KdToken2 *tok=frame_toks[i].toks;
        while( tok!=NULL )
        {
            DeleteForwardLinks(tok);
            KdToken2 *next_tok = tok->next;
            delete tok;
            num_toks_--;
            tok = next_tok;
        }
    }
    frame_toks.clear();
    KALDI_ASSERT(num_toks_ == 0);
}

// outputs a list in topological order
void KdLatticeDecoder::TopSortTokens(KdToken2 *tok_list, std::vector<KdToken2 *> *topsorted_list)
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

    for (IterType iter = token2pos.begin(); iter != token2pos.end(); ++iter)
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

    size_t max_loop = 1000000, loop_count; // max_loop is to detect epsilon cycles.
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
    KALDI_ASSERT(loop_count < max_loop && "Epsilon loops exist in your decoding "
               "graph (this is not allowed!)");

    topsorted_list->clear();
    topsorted_list->resize(cur_pos, NULL);  // create a list with NULLs in between.
    for (IterType iter = token2pos.begin(); iter != token2pos.end(); ++iter)
    {
        (*topsorted_list)[iter->second] = iter->first;
    }
}
// FinalRelativeCost()!=KD_INFINITY --> ReachedFinal

void KdLatticeDecoder::checkIntegrity(QString msg)
{
//    int end = frame_toks.size();
//    for( int f=0 ; f<end ; f++ )
//    {
//        int id_t = 0;
//        for( KdToken2 *tok=frame_toks[f].toks ; tok!=NULL ; tok=tok->next )
//        {
//            int id_l = 0;
//            KdToken2 *link;
//            KdFLink *flink = tok->links;
//            if( tok->is_linked==0 )
//            {
//                continue;
//            }
//            for ( link=tok->link_tok ; link!=NULL; link=link->link_tok )
//            {
//                if( link==NULL )
//                {
//                    break;
//                }
//                if( link!=flink->next_tok ) // emitting
//                {
//                    qDebug() << msg << "At frame" << f <<
//                             "ID_T" << id_t << "ID_L" << id_l;
//                }
//                flink = flink->next;
//                id_l++;
//            }
//            id_t++;
//        }
//    }
}
