#include "kd_faster_decoder.h"
#include <QDebug>

using namespace kaldi;

KdFasterDecoder::KdFasterDecoder(fst::Fst<fst::StdArc> &fst,
                                 FasterDecoderOptions &opts):
    fst_(fst), config_(opts), num_frames_decoded_(-1)
{
    KALDI_ASSERT(config_.hash_ratio >= 1.0);  // less doesn't make much sense.
    KALDI_ASSERT(config_.max_active > 1);
    KALDI_ASSERT(config_.min_active >= 0 && config_.min_active < config_.max_active);
    toks_.SetSize(1000);  // just so on the first frame we do something reasonable.
}


void KdFasterDecoder::InitDecoding()
{
    // clean up from last time:
    ClearToks(toks_.Clear());
    StateId start_state = fst_.Start();
    KALDI_ASSERT( start_state!=fst::kNoStateId );
    Arc dummy_arc(0, 0, Weight::One(), start_state);
    toks_.Insert(start_state, new KdFToken(dummy_arc, NULL));
    ProcessNonemitting(std::numeric_limits<float>::max());
    num_frames_decoded_ = 0;
}

void KdFasterDecoder::Decode(DecodableInterface *decodable)
{
    InitDecoding();
    AdvanceDecoding(decodable);
}

/// This will decode until there are no more frames ready in the decodable
/// object, but if max_num_frames is >= 0 it will decode no more than
/// that many frames.
void KdFasterDecoder::AdvanceDecoding(DecodableInterface *decodable,
                                      int32 max_num_frames)
{
    KALDI_ASSERT(num_frames_decoded_ >= 0 &&
                 "You must call InitDecoding() before AdvanceDecoding()");
    int32 num_frames_ready = decodable->NumFramesReady();
    // num_frames_ready must be >= num_frames_decoded, or else
    // the number of frames ready must have decreased (which doesn't
    // make sense) or the decodable object changed between calls
    // (which isn't allowed).
    KALDI_ASSERT(num_frames_ready >= num_frames_decoded_);
    int32 target_frames_decoded = num_frames_ready;
    if (max_num_frames >= 0)
    {
        target_frames_decoded = std::min(target_frames_decoded,
                                         num_frames_decoded_ + max_num_frames);
    }
    while (num_frames_decoded_ < target_frames_decoded)
    {
        // note: ProcessEmitting() increments num_frames_decoded_
        double weight_cutoff = ProcessEmitting(decodable);
        ProcessNonemitting(weight_cutoff);
    }
}

int32 KdFasterDecoder::NumFramesDecoded()
{
    return num_frames_decoded_;
}

bool KdFasterDecoder::ReachedFinal()
{
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    {
        if( e->val->cost!=KD_INFINITY && fst_.Final(e->key)!=Weight::Zero() )
        {
            return true;
        }
    }
    return false;
}

/// GetBestPath gets the decoding traceback. If "use_final_probs" is true
/// AND we reached a final state, it limits itself to final states;
/// otherwise it gets the most likely token not taking into account
/// final-probs. Returns true if the output best path was not the empty
/// FST (will only return false in unusual circumstances where
/// no tokens survived).
bool KdFasterDecoder::GetBestPath(fst::MutableFst<LatticeArc> *fst_out,
                                  bool use_final_probs)
{
    fst_out->DeleteStates();
    KdFToken *best_tok = NULL;
    bool is_final = ReachedFinal();
    if (!is_final)
    {
        for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
        {
            if (best_tok == NULL || *best_tok < *(e->val) )
                best_tok = e->val;
        }
    }
    else
    {
        double best_cost = KD_INFINITY;
        for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
        {
            double this_cost = e->val->cost + fst_.Final(e->key).Value();
            if( this_cost<best_cost && this_cost!=KD_INFINITY )
            {
                best_cost = this_cost;
                best_tok = e->val;
            }
        }
    }
    if( best_tok==NULL)
    {
        return false;  // No output.
    }

    std::vector<LatticeArc> arcs_reverse;  // arcs in reverse order.

    for (KdFToken *tok = best_tok; tok != NULL; tok = tok->prev_) {
        BaseFloat tot_cost = tok->cost -
                (tok->prev_ ? tok->prev_->cost : 0.0),
                graph_cost = tok->arc_.weight.Value(),
                ac_cost = tot_cost - graph_cost;
        LatticeArc l_arc(tok->arc_.ilabel,
                         tok->arc_.olabel,
                         LatticeWeight(graph_cost, ac_cost),
                         tok->arc_.nextstate);
        arcs_reverse.push_back(l_arc);
    }
    KALDI_ASSERT(arcs_reverse.back().nextstate == fst_.Start());
    arcs_reverse.pop_back();  // that was a "fake" token... gives no info.

    StateId cur_state = fst_out->AddState();
    fst_out->SetStart(cur_state);
    for (ssize_t i = static_cast<ssize_t>(arcs_reverse.size())-1; i >= 0; i--) {
        LatticeArc arc = arcs_reverse[i];
        arc.nextstate = fst_out->AddState();
        fst_out->AddArc(cur_state, arc);
        cur_state = arc.nextstate;
    }
    if (is_final && use_final_probs)
    {
        Weight final_weight = fst_.Final(best_tok->arc_.nextstate);
        fst_out->SetFinal(cur_state, LatticeWeight(final_weight.Value(), 0.0));
    }
    else
    {
        fst_out->SetFinal(cur_state, LatticeWeight::One());
    }
    RemoveEpsLocal(fst_out);
    return true;
}


// Gets the weight cutoff.  Also counts the active tokens.
double KdFasterDecoder::GetCutoff(Elem *list_head, size_t *tok_count,
                                  float *adaptive_beam, Elem **best_elem)
{
    double best_cost = KD_INFINITY;
    size_t count = 0;

    tmp_array_.clear();
    for( Elem *e=list_head ; e!=NULL ; e=e->tail )
    {
        count++;
        double w = e->val->cost;
        tmp_array_.push_back(w);
//        qDebug() << count << w;

        if( w<best_cost )
        {
            best_cost = w;
            if( best_elem )
            {
                *best_elem = e;
            }
        }
    }
    if (tok_count != NULL)
    {
        *tok_count = count;
    }


    double beam_cutoff = best_cost + config_.beam;
    double min_active_cutoff = KD_INFINITY;
    double max_active_cutoff = KD_INFINITY;

    if( tmp_array_.size()>config_.max_active )
    {
        std::nth_element(tmp_array_.begin(),
                         tmp_array_.begin() + config_.max_active,
                         tmp_array_.end());
        max_active_cutoff = tmp_array_[config_.max_active];
    }
    if(max_active_cutoff < beam_cutoff)
    { // max_active is tighter than beam.
        if (adaptive_beam)
        {
            *adaptive_beam = max_active_cutoff - best_cost + config_.beam_delta;
        }
        return max_active_cutoff;
    }
    if( tmp_array_.size()>config_.min_active )
    {
        if (config_.min_active == 0)
        {
            min_active_cutoff = best_cost;
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
        {
            *adaptive_beam = min_active_cutoff - best_cost + config_.beam_delta;
        }
        return min_active_cutoff;
    }
    else
    {
//        qDebug() << "best_cost" << beam_cutoff;
        *adaptive_beam = config_.beam;
        return beam_cutoff;
    }
}

double KdFasterDecoder::GetBestCutoff(Elem *best_elem, DecodableInterface *decodable,
                                      float adaptive_beam)
{
    double cutoff = KD_INFINITY;
    if( best_elem )
    {
        StateId state = best_elem->key;
        KdFToken *tok = best_elem->val;
        for( fst::ArcIterator<fst::Fst<Arc> > aiter(fst_, state) ;
             !aiter.Done(); aiter.Next())
        {
            const Arc &arc = aiter.Value();
            if( arc.ilabel!=0 )
            {
                BaseFloat ac_cost = - decodable->LogLikelihood(num_frames_decoded_, arc.ilabel);
                double new_weight = arc.weight.Value() + tok->cost + ac_cost + adaptive_beam;
                if( new_weight<cutoff )
                {
                    cutoff = new_weight + adaptive_beam;
                }
            }
        }
    }
    return cutoff;
}

// ProcessEmitting returns the likelihood cutoff used.
// It decodes the frame num_frames_decoded_ of the decodable object
// and then increments num_frames_decoded_
double KdFasterDecoder::ProcessEmitting(DecodableInterface *decodable)
{
    int32 frame = num_frames_decoded_;
//    ClearToks(toks_.Clear());
    Elem *last_toks = toks_.Clear();
    size_t tok_cnt;
    float adaptive_beam;
    Elem *best_elem = NULL;
    double cutoff = GetCutoff(last_toks, &tok_cnt,
                                     &adaptive_beam, &best_elem);

    // This is the cutoff we use after adding in the log-likes (i.e.
    // for the next frame).  This is a bound on the cutoff we will use
    // on the next frame.
    double next_weight_cutoff = GetBestCutoff(best_elem, decodable,
                                              adaptive_beam);

    Elem *e_tail;
//    qDebug() << "tok_count" << tok_cnt;
    int tok_count = 0;
    for( Elem *e=last_toks ; e!=NULL ; e=e_tail )
    {
        tok_count++;

        StateId state = e->key;
        KdFToken *tok = e->val;
        if( tok->cost<cutoff )
        {
            KALDI_ASSERT(state == tok->arc_.nextstate);
            for (fst::ArcIterator<KdFST> aiter(fst_, state); !aiter.Done(); aiter.Next())
            {
                Arc arc = aiter.Value();
                if (arc.ilabel != 0)
                {
                    BaseFloat ac_cost =  - decodable->LogLikelihood(frame, arc.ilabel);
                    double new_weight = arc.weight.Value() + tok->cost + ac_cost;
                    if (new_weight < next_weight_cutoff)
                    {  // not pruned..
                        KdFToken *new_tok = new KdFToken(arc, ac_cost, tok);
                        Elem *e_found = toks_.Insert(arc.nextstate, new_tok);
                        if (new_weight + adaptive_beam < next_weight_cutoff)
                        {
                            next_weight_cutoff = new_weight + adaptive_beam;
                        }
                        if (e_found->val != new_tok)
                        {
                            if (*(e_found->val) < *new_tok)
                            {
                                KdFTokenDelete(e_found->val);
                                e_found->val = new_tok;
                            }
                            else
                            {
                                KdFTokenDelete(new_tok);
                            }
                        }
                    }
                }
            }
        }
        e_tail = e->tail;
        KdFTokenDelete(e->val);
        toks_.Delete(e);
    }
    num_frames_decoded_++;
    return next_weight_cutoff;
}

void KdFasterDecoder::ProcessNonemitting(double cutoff)
{
    // Processes nonemitting arcs for one frame.
    KALDI_ASSERT(queue_.empty());
    for (const Elem *e = toks_.GetList(); e != NULL;  e = e->tail)
    {
        queue_.push_back(e);
    }

    while (!queue_.empty())
    {
        const Elem* e = queue_.back();
        queue_.pop_back();
        StateId state = e->key;
        KdFToken *tok = e->val;  // would segfault if state not
        // in toks_ but this can't happen.
        if (tok->cost > cutoff)
        { // Don't bother processing successors.
            continue;
        }
        KALDI_ASSERT(tok != NULL && state == tok->arc_.nextstate);
        for (fst::ArcIterator<fst::Fst<Arc> > aiter(fst_, state);
             !aiter.Done(); aiter.Next())
        {
            Arc *arc = const_cast<Arc *>(&(aiter.Value()));
            if ( arc->ilabel==0 )
            {  // propagate nonemitting only...
                KdFToken *new_tok = new KdFToken(*arc, tok);

                if( new_tok->cost>cutoff )
                {  // prune
                    KdFTokenDelete(new_tok);
                }
                else
                {
                    Elem *e_found = toks_.Insert(arc->nextstate, new_tok);
                    if (e_found->val == new_tok)
                    {
                        queue_.push_back(e_found);
                    }
                    else
                    {
                        if (*(e_found->val) < *new_tok)
                        {
                            KdFTokenDelete(e_found->val);
                            e_found->val = new_tok;
                            queue_.push_back(e_found);
                        }
                        else
                        {
                            KdFTokenDelete(new_tok);
                        }
                    }
                }
            }
        }
    }
}

// It might seem unclear why we call ClearToks(toks_.Clear()).
// There are two separate cleanup tasks we need to do at when we start a new file.
// one is to delete the KdFToken objects in the list; the other is to delete
// the Elem objects.  toks_.Clear() just clears them from the hash and gives ownership
// to the caller, who then has to call toks_.Delete(e) for each one.  It was designed
// this way for convenience in propagating tokens from one frame to the next.
void KdFasterDecoder::ClearToks(Elem *list)
{
    Elem *e_tail;
    for( Elem *e=list ; e!=NULL ; e=e_tail )
    {
        KdFTokenDelete(e->val);
        e_tail = e->tail;
        toks_.Delete(e);
    }
}

void KdFasterDecoder::SetOptions(kaldi::FasterDecoderOptions &config)
{
    config_ = config;
}

KdFasterDecoder::~KdFasterDecoder()
{
    ClearToks(toks_.Clear());
}
