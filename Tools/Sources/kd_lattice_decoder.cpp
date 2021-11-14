#include "kd_lattice_decoder.h"
#include <QDebug>
#include "lat/lattice-functions.h"

using namespace kaldi;

KdLatticeDecoder::KdLatticeDecoder(KdFST &fst, LatticeFasterDecoderConfig &config):
    fst_(&fst), config_(config), num_toks_(0)
{
    config.Check();
    toks_.SetSize(1000);  // just so on the first frame we do something reasonable.
}

KdLatticeDecoder::~KdLatticeDecoder()
{
    DeleteElems(toks_.Clear());
    ClearActiveTokens();
}

void KdLatticeDecoder::InitDecoding()
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
    KdToken *start_tok = new KdToken(0.0, 0.0, NULL, NULL, NULL);
    frame_toks[0].toks = start_tok;
    toks_.Insert(start_state, start_tok);
    num_toks_++;
    ProcessNonemitting(config_.beam);
}

// return false indicates error.
bool KdLatticeDecoder::Decode(DecodableInterface *decodable)
{
    InitDecoding();
    AdvanceDecoding(decodable);
    FinalizeDecoding();

    return !frame_toks.empty() && frame_toks.back().toks != NULL;
}

/// Returns true if result is nonempty.
bool KdLatticeDecoder::GetBestPath(Lattice *olat, bool use_final_probs)
{
    Lattice raw_lat;
    GetRawLattice(&raw_lat, use_final_probs);
    ShortestPath(raw_lat, olat);
    return (olat->NumStates() != 0);
}

bool KdLatticeDecoder::GetRawLattice(Lattice *ofst, bool use_final_probs)
{
    if (decoding_finalized_ && !use_final_probs)
    {
        KALDI_ERR << "You cannot call FinalizeDecoding() and then call "
              << "GetRawLattice() with use_final_probs == false";
    }

    unordered_map<KdToken*, float> final_costs_local;

    const unordered_map<KdToken*, float> &final_costs =
            (decoding_finalized_ ? final_costs_ : final_costs_local);
    if (!decoding_finalized_ && use_final_probs)
    {
        ComputeFinalCosts(&final_costs_local, NULL, NULL);
    }

    ofst->DeleteStates();
    // num-frames plus one (since frames are one-based, and we have
    // an extra frame for the start-state).
    int32 num_frames = frame_toks.size() - 1;

    if( num_frames<=0 )
    {
        return false;
    }

    const int32 bucket_count = num_toks_/2 + 3;
    unordered_map<KdToken*, KdStateId> tok_map(bucket_count);
    // First create all states.
    std::vector<KdToken*> token_list;
    for (int32 f = 0; f <= num_frames; f++)
    {
        if (frame_toks[f].toks == NULL)
        {
            KALDI_WARN << "GetRawLattice: no tokens active on frame " << f
                       << ": not producing lattice.\n";
            return false;
        }
        TopSortTokens(frame_toks[f].toks, &token_list);
        for (size_t i = 0; i < token_list.size(); i++)
            if (token_list[i] != NULL)
                tok_map[token_list[i]] = ofst->AddState();
    }
    // The next statement sets the start state of the output FST.  Because we
    // topologically sorted the tokens, state zero must be the start-state.
    ofst->SetStart(0);

    KALDI_VLOG(4) << "init:" << num_toks_/2 + 3 << " buckets:"
                << tok_map.bucket_count() << " load:" << tok_map.load_factor()
                << " max:" << tok_map.max_load_factor();
    // Now create all arcs.
    for (int32 f = 0; f <= num_frames; f++)
    {
        for (KdToken *tok = frame_toks[f].toks; tok != NULL; tok = tok->next)
        {
            KdStateId cur_state = tok_map[tok];
            for ( KdFLink *l = tok->links; l != NULL; l = l->next )
            {
                typename unordered_map<KdToken*, KdStateId>::const_iterator
                        iter = tok_map.find(l->next_tok);
                KdStateId nextstate = iter->second;
                KALDI_ASSERT(iter != tok_map.end());
                float cost_offset = 0.0;
                if (l->ilabel != 0)
                {  // emitting..
                    KALDI_ASSERT(f >= 0 && f<cost_offsets.length() );
                    cost_offset = cost_offsets[f];
                }
                LatticeArc::Weight arc_w(l->graph_cost, l->acoustic_cost - cost_offset);
                LatticeArc arc(l->ilabel, l->olabel,arc_w, nextstate);
                ofst->AddArc(cur_state, arc);
            }
            if (f == num_frames)
            {
                if (use_final_probs && !final_costs.empty())
                {
                    typename unordered_map<KdToken*, float>::const_iterator
                            iter = final_costs.find(tok);
                    if (iter != final_costs.end())
                        ofst->SetFinal(cur_state, LatticeWeight(iter->second, 0));
                }
                else
                {
                    ofst->SetFinal(cur_state, LatticeWeight::One());
                }
            }
        }
    }
    return (ofst->NumStates() > 0);
}

void KdLatticeDecoder::PossiblyResizeHash(size_t num_toks)
{
    size_t new_sz = static_cast<size_t>(static_cast<float>(num_toks)
                                        * config_.hash_ratio);
    if (new_sz > toks_.Size()) {
        toks_.SetSize(new_sz);
    }
}

// Locates a token in toks_, or inserts a new, empty token
// for the current frame. 'changed' true if a token created or cost changed.
KdLatticeDecoder::Elem* KdLatticeDecoder::FindOrAddToken(
        KdStateId state, int32 frame_plus_one, float tot_cost,
        KdToken *backpointer, bool *changed)
{
    KALDI_ASSERT(frame_plus_one < frame_toks.size());
    Elem *e_found = toks_.Insert(state, NULL);
    if( e_found->val==NULL )
    {
        KdTokenList *tok_list = &frame_toks[frame_plus_one];
        const float extra_cost = 0.0;
        // tokens on the currently final frame have zero extra_cost
        // as any of them could end up on the winning path.
        KdToken *new_tok = new KdToken (tot_cost, extra_cost, NULL, tok_list->toks, backpointer);
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
    else
    {
        KdToken *tok = e_found->val;
        if( tok->tot_cost > tot_cost ) // if there is an existing token for this state.
        {  // replace old token
            tok->tot_cost = tot_cost;
            tok->SetBackpointer(backpointer);
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

// prunes outgoing links for all tokens in frame_toks[frame]
// PruneActiveTokens
bool KdLatticeDecoder::PruneForwardLinks(
        int32 frame, bool *extra_costs_changed, float delta)
{
    // If delta is larger,  we'll go back less toward the beginning

    *extra_costs_changed = false;
    bool links_pruned = false;
    KALDI_ASSERT(frame>=0 && frame<frame_toks.size());
    KdToken *start_tok = frame_toks[frame].toks;

    if( frame_toks[frame].toks==NULL )
    {  // empty list; should not happen.
        if( !warned_ )
        {
            qDebug() << "PruneForwardLinks: No tokens alive @ " << frame;
            warned_ = true;
        }
    }

    bool changed = true;  // difference new minus old extra cost >= delta ?
    while( changed )
    {
        changed = false;
        for( KdToken *tok=start_tok ; tok!=NULL; tok=tok->next )
        {
            KdFLink *link, *prev_link = NULL;
            // will recompute tok_extra_cost for tok.
            float tok_extra_cost = std::numeric_limits<float>::infinity();
            // tok_extra_cost is the best (min) of link_extra_cost of outgoing links
            for( link = tok->links; link != NULL; )
            {
                // See if we need to excise this link...
                KdToken *next_tok = link->next_tok;
                float link_extra_cost = next_tok->extra_cost +
                        ((tok->tot_cost + link->acoustic_cost + link->graph_cost)
                         - next_tok->tot_cost);  // difference in brackets is >= 0
                // link_exta_cost is the difference in score between the best paths
                // through link source state and through link destination state
                KALDI_ASSERT(link_extra_cost == link_extra_cost);  // check for NaN
                if( link_extra_cost>config_.lattice_beam )  // delete link
                {
                    KdFLink *next_link = link->next;
                    if (prev_link != NULL) prev_link->next = next_link;
                    else tok->links = next_link;
                    delete link;
                    link = next_link;  // advance link but leave prev_link the same.
                    links_pruned = true;
                }
                else
                {   // keep the link and update the tok_extra_cost if needed.
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
            }  // for all outgoing links
            if( fabs(tok_extra_cost-tok->extra_cost ) > delta)
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
        }  // for all KdToken on frame_toks[frame]
        if (changed)
        {
            *extra_costs_changed = true;
        }

    }

    return links_pruned;
}

// PruneForwardLinksFinal is a version of PruneForwardLinks that we call
// on the final frame.  If there are final tokens active, it uses
// the final-probs for pruning, otherwise it treats all tokens as final.
void KdLatticeDecoder::PruneForwardLinksFinal()
{
    KALDI_ASSERT(!frame_toks.empty());
    int32 frame_plus_one = frame_toks.size() - 1;

    if (frame_toks[frame_plus_one].toks == NULL)  // empty list; should not happen.
        KALDI_WARN << "No tokens alive at end of file";

    typedef typename unordered_map<KdToken*, float>::const_iterator IterType;
    ComputeFinalCosts(&final_costs_, &final_relative_cost_, &final_best_cost_);
    decoding_finalized_ = true;
    // We call DeleteElems() as a nicety, not because it's really necessary;
    // otherwise there would be a time, after calling PruneTokensForFrame() on the
    // final frame, when toks_.GetList() or toks_.Clear() would contain pointers
    // to nonexistent tokens.
    DeleteElems(toks_.Clear());

    // Now go through tokens on this frame, pruning forward links...  may have to
    // iterate a few times until there is no more change, because the list is not
    // in topological order.  This is a modified version of the code in
    // PruneForwardLinks, but here we also take account of the final-probs.
    bool changed = true;
    float delta = 1.0e-05;
    while (changed) {
        changed = false;
        for (KdToken *tok = frame_toks[frame_plus_one].toks;
             tok != NULL; tok = tok->next) {
            KdFLink *link, *prev_link = NULL;
            // will recompute tok_extra_cost.  It has a term in it that corresponds
            // to the "final-prob", so instead of initializing tok_extra_cost to infinity
            // below we set it to the difference between the (score+final_prob) of this token,
            // and the best such (score+final_prob).
            float final_cost;
            if (final_costs_.empty()) {
                final_cost = 0.0;
            } else {
                IterType iter = final_costs_.find(tok);
                if (iter != final_costs_.end())
                    final_cost = iter->second;
                else
                    final_cost = std::numeric_limits<float>::infinity();
            }
            float tok_extra_cost = tok->tot_cost + final_cost - final_best_cost_;
            // tok_extra_cost will be a "min" over either directly being final, or
            // being indirectly final through other links, and the loop below may
            // decrease its value:
            for (link = tok->links; link != NULL; ) {
                // See if we need to excise this link...
                KdToken *next_tok = link->next_tok;
                float link_extra_cost = next_tok->extra_cost +
                        ((tok->tot_cost + link->acoustic_cost + link->graph_cost)
                         - next_tok->tot_cost);
                if (link_extra_cost > config_.lattice_beam) {  // excise link
                    KdFLink *next_link = link->next;
                    if (prev_link != NULL) prev_link->next = next_link;
                    else tok->links = next_link;
                    delete link;
                    link = next_link; // advance link but leave prev_link the same.
                } else { // keep the link and update the tok_extra_cost if needed.
                    if (link_extra_cost < 0.0) { // this is just a precaution.
                        if (link_extra_cost < -0.01)
                            KALDI_WARN << "Negative extra_cost: " << link_extra_cost;
                        link_extra_cost = 0.0;
                    }
                    if (link_extra_cost < tok_extra_cost)
                        tok_extra_cost = link_extra_cost;
                    prev_link = link;
                    link = link->next;
                }
            }
            // prune away tokens worse than lattice_beam above best path.  This step
            // was not necessary in the non-final case because then, this case
            // showed up as having no forward links.  Here, the tok_extra_cost has
            // an extra component relating to the final-prob.
            if (tok_extra_cost > config_.lattice_beam)
                tok_extra_cost = std::numeric_limits<float>::infinity();
            // to be pruned in PruneTokensForFrame

            if (!ApproxEqual(tok->extra_cost, tok_extra_cost, delta))
                changed = true;
            tok->extra_cost = tok_extra_cost; // will be +infinity or <= lattice_beam_.
        }
    } // while changed
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
void KdLatticeDecoder::PruneTokensForFrame(int32 frame)
{
    KALDI_ASSERT( frame>=0 && frame<frame_toks.size() );

    KdToken *&start_token = frame_toks[frame].toks;
    if( start_token==NULL )
    {
        qDebug() << "PruneTokensForFrame: No tokens alive @" << frame;
    }

    KdToken *tok, *next_tok, *prev_tok = NULL;
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
    int32 cur_frame_plus_one = frame_toks.size();
    int32 num_toks_begin = num_toks_;

    for(int f=cur_frame_plus_one-1 ; f>=0 ; f-- )
    {
        // Reason why we need to prune forward links in this situation:
        // (1) we have never pruned them (new TokenList)
        // (2) we have not yet pruned the forward links to the next f,
        // after any of those tokens have changed their extra_cost.
        if( frame_toks[f].must_prune_forward_links )
        {
            bool extra_costs_changed = false;
            bool links_pruned;
            links_pruned = PruneForwardLinks(f, &extra_costs_changed, delta);

            if( extra_costs_changed && f>0 ) // any token has changed extra_cost
            {
                frame_toks[f-1].must_prune_forward_links = true;
            }
            if (links_pruned) // any link was pruned
            {
                frame_toks[f].must_prune_tokens = true;
            }
            frame_toks[f].must_prune_forward_links = false; // job done
        }

        if( f+1<cur_frame_plus_one && frame_toks[f+1].must_prune_tokens )
        {
            PruneTokensForFrame(f+1);
            frame_toks[f+1].must_prune_tokens = false;
        }
    }
    KALDI_VLOG(4) << "PruneActiveTokens: pruned tokens from " << num_toks_begin
                  << " to " << num_toks_;
}

// computes final-costs for the final frame. It outputs to final-costs, a map from the KdToken*
// pointer to the final-prob of the corresponding state, for all Tokens
// that correspond to states that have final-probs.
void KdLatticeDecoder::ComputeFinalCosts(unordered_map<KdToken *, float> *final_costs,
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
        KdToken *tok = final_toks->val;
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
    if (final_relative_cost != NULL) {
        if (best_cost == infinity && best_cost_with_final == infinity) {
            // Likely this will only happen if there are no tokens surviving.
            // This seems the least bad way to handle it.
            *final_relative_cost = infinity;
        } else {
            *final_relative_cost = best_cost_with_final - best_cost;
        }
    }
    if (final_best_cost != NULL) {
        if (best_cost_with_final != infinity) { // final-state exists.
            *final_best_cost = best_cost_with_final;
        } else { // no final-state exists.
            *final_best_cost = best_cost;
        }
    }
}

void KdLatticeDecoder::AdvanceDecoding(DecodableInterface *decodable)
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
        float cost_cutoff = ProcessEmitting(decodable);
        ProcessNonemitting(cost_cutoff);
    }
}

// optionally called, does extra pruning
void KdLatticeDecoder::FinalizeDecoding()
{
    int32 final_frame_plus_one = frame_toks.size()-1;
    int32 num_toks_begin = num_toks_;

    for (int32 f = final_frame_plus_one - 1; f >= 0; f--)
    {
        bool b1; // values not used.
        float dontcare = 0.0; // delta of zero means we must always update
        PruneForwardLinks(f, &b1, dontcare);
        PruneTokensForFrame(f + 1);
    }
    PruneTokensForFrame(0);
    KALDI_VLOG(4) << "pruned tokens from " << num_toks_begin
                  << " to " << num_toks_;
}

float KdLatticeDecoder::GetCutoff(Elem *list_head, size_t *tok_count,
                                  float *adaptive_beam, Elem **best_elem)
{
    float best_weight = std::numeric_limits<float>::infinity();
    // positive == high cost == bad.
    size_t count = 0;
    if (config_.max_active == std::numeric_limits<int32>::max() &&
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
        if (adaptive_beam != NULL)
        {
            *adaptive_beam = config_.beam;
        }
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
            if (adaptive_beam)
            {
                *adaptive_beam = max_active_cutoff - best_weight + config_.beam_delta;
            }
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
            if (adaptive_beam)
                *adaptive_beam = min_active_cutoff - best_weight + config_.beam_delta;
            return min_active_cutoff;
        } else {
            *adaptive_beam = config_.beam;
            return beam_cutoff;
        }
    }
}

double KdLatticeDecoder::GetBestCutoff(Elem *best_elem, DecodableInterface *decodable,
                                      float adaptive_beam)
{
    double cutoff = KD_INFINITY;
    int32 frame = cost_offsets.size();

    cost_offsets.push_back(0.0);
    if( best_elem )
    {
        KdStateId state = best_elem->key;
        KdToken *tok = best_elem->val;
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
float KdLatticeDecoder::ProcessEmitting(DecodableInterface *decodable)
{
    KALDI_ASSERT(frame_toks.size() > 0);
    int32 frame = frame_toks.size() - 1;
    frame_toks.resize(frame_toks.size() + 1);

    Elem *final_toks = toks_.Clear();
    Elem *best_elem = NULL;
    float adaptive_beam;
    size_t tok_cnt;

    float cutoff = GetCutoff(final_toks, &tok_cnt, &adaptive_beam, &best_elem);
    float next_cutoff = GetBestCutoff(best_elem, decodable, adaptive_beam);

    Elem *e_tail;
    for( Elem *e=final_toks ; e!=NULL ; e=e_tail )
    {
        KdStateId state = e->key;
        KdToken *tok = e->val;
        if( tok->tot_cost<=cutoff )
        {
            for(fst::ArcIterator<KdFST> aiter(*fst_, state); !aiter.Done(); aiter.Next() )
            {
                const KdArc &arc = aiter.Value();
                if (arc.ilabel != 0)
                {
                    float new_weight = decodable->LogLikelihood(frame_num, arc.ilabel);
                    float ac_cost = cost_offsets[frame] - new_weight;
                    float graph_cost = arc.weight.Value();
                    float tot_cost = tok->tot_cost + ac_cost + graph_cost;

                    if( tot_cost>=next_cutoff )
                    {
                        continue;
                    }
                    else if( tot_cost+adaptive_beam<next_cutoff )
                    {
                        next_cutoff = tot_cost + adaptive_beam; // prune by best current token
                    }

                    Elem *e_next = FindOrAddToken(arc.nextstate,
                                                  frame + 1, tot_cost, tok, NULL);

                    // Add ForwardLink from tok to next_tok (put on head of list tok->links)
                    tok->links = new KdFLink(e_next->val, arc.ilabel, arc.olabel,
                                             graph_cost, ac_cost, tok->links);
                }
            }
        }
        e_tail = e->tail;
        toks_.Delete(e);
    }

    frame_num++;
    return next_cutoff;
}

// Processes for one frame.
void KdLatticeDecoder::ProcessNonemitting(float cutoff)
{
    KALDI_ASSERT(!frame_toks.empty());
    int32 frame = static_cast<int32>(frame_toks.size()) - 2;
    // Note: "frame" is the time-index we just processed, or -1 if
    // we are processing the nonemitting transitions before the
    // first frame (called from InitDecoding()).

    // Processes nonemitting arcs for one frame.  Propagates within toks_.
    // Note-- this queue structure is not very optimal as
    // it may cause us to process states unnecessarily (e.g. more than once),
    // but in the baseline code, turning this vector into a set to fix this
    // problem did not improve overall speed.

    KALDI_ASSERT(queue_.empty());

    if (toks_.GetList() == NULL)
    {
        if (!warned_)
        {
            KALDI_WARN << "Error, no surviving tokens: frame is " << frame;
            warned_ = true;
        }
    }

    for (Elem *e = const_cast<Elem *>(toks_.GetList()); e != NULL;  e = e->tail)
    {
        KdStateId state = e->key;
        if (fst_->NumInputEpsilons(state) != 0)
        {
            queue_.push_back(e);
        }
    }

    while( !queue_.empty() )
    {
        const Elem *e = queue_.back();
        queue_.pop_back();

        KdStateId state = e->key;
        KdToken *tok = e->val;  // would segfault if e is a NULL pointer but this can't happen.
        float cur_cost = tok->tot_cost;
        if( cur_cost>=cutoff ) // Don't bother processing successors.
        {
            continue;
        }
        // If "tok" has any existing forward links, delete them,
        // because we're about to regenerate them.  This is a kind
        // of non-optimality (remember, this is the simple decoder),
        // but since most states are emitting it's not a huge issue.
        DeleteForwardLinks(tok); // necessary when re-visiting
        tok->links = NULL;
        for(fst::ArcIterator<KdFST> aiter(*fst_, state); !aiter.Done(); aiter.Next())
        {
            const KdArc &arc = aiter.Value();
            if (arc.ilabel == 0)
            {  // propagate nonemitting only...
                float graph_cost = arc.weight.Value();
                float tot_cost = cur_cost + graph_cost;
                if (tot_cost < cutoff)
                {
                    bool changed;
                    Elem *e_new = FindOrAddToken(arc.nextstate, frame + 1, tot_cost,
                                                 tok, &changed);

                    tok->links = new KdFLink(e_new->val, 0, arc.olabel,
                                                  graph_cost, 0, tok->links);

                    // "changed" tells us whether the new token has a different
                    // cost from before, or is new [if so, add into queue].
                    if (changed && fst_->NumInputEpsilons(arc.nextstate) != 0)
                    {
                        queue_.push_back(e_new);
                    }
                }
            }
        } // for all arcs
    } // while queue not empty
}

// Deletes the elements of the singly linked list tok->links.
void KdLatticeDecoder::DeleteForwardLinks(KdToken *tok)
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
        KdToken *tok=frame_toks[i].toks;
        while( tok!=NULL )
        {
            DeleteForwardLinks(tok);
            KdToken *next_tok = tok->next;
            delete tok;
            num_toks_--;
            tok = next_tok;
        }
    }
    frame_toks.clear();
    KALDI_ASSERT(num_toks_ == 0);
}

// outputs a list in topological order
void KdLatticeDecoder::TopSortTokens(
        KdToken *tok_list, std::vector<KdToken *> *topsorted_list)
{
    unordered_map<KdToken*, int32> token2pos;
    typedef typename unordered_map<KdToken*, int32>::iterator IterType;
    int32 num_toks = 0;
    for (KdToken *tok = tok_list; tok != NULL; tok = tok->next)
        num_toks++;
    int32 cur_pos = 0;
    // We assign the tokens numbers num_toks - 1, ... , 2, 1, 0.
    // This is likely to be in closer to topological order than
    // if we had given them ascending order, because of the way
    // new tokens are put at the front of the list.
    for (KdToken *tok = tok_list; tok != NULL; tok = tok->next)
    {
        token2pos[tok] = num_toks - ++cur_pos;
    }

    unordered_set<KdToken*> reprocess;

    for (IterType iter = token2pos.begin(); iter != token2pos.end(); ++iter)
    {
        KdToken *tok = iter->first;
        int32 pos = iter->second;
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
                    int32 next_pos = following_iter->second;
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
        std::vector<KdToken*> reprocess_vec;
        for (typename unordered_set<KdToken*>::iterator iter = reprocess.begin();
             iter != reprocess.end(); ++iter)
            reprocess_vec.push_back(*iter);
        reprocess.clear();
        for (typename std::vector<KdToken*>::iterator iter = reprocess_vec.begin();
             iter != reprocess_vec.end(); ++iter) {
            KdToken *tok = *iter;
            int32 pos = token2pos[tok];
            // Repeat the processing we did above (for comments, see above).
            for (KdFLink *link = tok->links; link != NULL; link = link->next) {
                if (link->ilabel == 0) {
                    IterType following_iter = token2pos.find(link->next_tok);
                    if (following_iter != token2pos.end()) {
                        int32 next_pos = following_iter->second;
                        if (next_pos < pos) {
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

