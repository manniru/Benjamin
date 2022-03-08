#include "kd_online_ldecoder.h"
#include <QDebug>

using namespace kaldi;
QString dbg_times;
#define BT_MIN_SIL 9 //100ms ((x+1)*100)

KdOnlineLDecoder::KdOnlineLDecoder(QVector<int> sil_phones,
                                   kaldi::TransitionModel &trans_model):
    trans_model_(trans_model)
{
    fst_ = kd_readDecodeGraph(BT_FST_PATH);

    opts.max_active = 7000;
    opts.lattice_beam = 6.0;

    config_ = opts;

    silence_set = sil_phones;
    uframe = 0;
    effective_beam_ = opts.beam;
    start_t = clock();
}

//frame_num would not reset
void KdOnlineLDecoder::ResetDecoder()
{
//    qDebug() << "Reset Kaldi" << full << frame_toks.size();
    DeleteElems(elements.Clear()); //replaced ClearToks
    cost_offsets.clear();
    ClearActiveTokens(); ///THIS LINE SHOULD NOT EXECUTED!
    final_costs_.clear();

    KdStateId start_state = fst_->Start();
    frame_toks.resize(1);
    KdToken2 *start_tok = new KdToken2(0.0, 0.0, NULL);
    frame_toks[0].toks = start_tok;
    elements.Insert(start_state, start_tok);
    num_elements = 1;
    uframe = 0;
    ProcessNonemitting(config_.beam);
}

void KdOnlineLDecoder::RawLattice(KdLattice *ofst)
{
    int end = frame_toks.size();
    unordered_map<KdToken2*, float> final_costs;
    ComputeFinalCosts(&final_costs, NULL, NULL);
    dbg_times += " F:";
    dbg_times += getLDiffTime();
    createStates();
    *ofst = cache_fst1;
    dbg_times += " C:";
    dbg_times += getLDiffTime();

    // Now create all arcs.
    for( int f=0 ; f<end ; f++ )
    {
        for( KdToken2 *tok=frame_toks[f].toks ; tok!=NULL ; tok=tok->next )
        {
            KdStateId state = tok->state;
            QString dbg_buf;
            KdFLink *link;
            for ( link=tok->links; link!=NULL; link=link->next )
            {
                KdStateId nextstate = link->next_tok->state;

                float cost_offset = 0.0;
                if( link->ilabel!=0 ) // emitting
                {
                    KALDI_ASSERT(f >= 0 && f<cost_offsets.length() );
                    cost_offset = cost_offsets[f];
                }
                dbg_buf += "[";
                dbg_buf += QString::number(link->ilabel);
                dbg_buf += ",";
                dbg_buf += QString::number(link->acoustic_cost);
                dbg_buf += ",";
                dbg_buf += QString::number(cost_offset);
                dbg_buf += ",";
                dbg_buf += QString::number(link->graph_cost);
                dbg_buf += "->";
                KdLatticeArc::Weight arc_w(link->graph_cost, link->acoustic_cost - cost_offset);
                KdLatticeArc arc(link->ilabel, link->olabel,arc_w, nextstate);
                ofst->AddArc(state, arc);
            }
            if( f==end-1 )
            {
                if( !final_costs.empty() )
                {
                    unordered_map<KdToken2*, float>::const_iterator
                            iter = final_costs.find(tok);
                    if( iter!=final_costs.end() )
                    {
                        qDebug() << "FF---->" << iter->second;
//                        LatticeWeight w = LatticeWeight(iter->second, 0);
                        ofst->SetFinal(state, LatticeWeight(iter->second, 0));
                    }
                }
                else
                {
                    ofst->SetFinal(state, LatticeWeight::One());
                }
            }

//            qDebug() << "RL---->" << state
//                     << "u" << f;
//            qDebug() << dbg_buf;
        }
    }
}

void KdOnlineLDecoder::createStates()
{
    int end = frame_toks.size();
    cache_fst1.DeleteStates();
    last_cache_f = 0;

    // First create all states.
    std::vector<KdToken2*> token_list;
    for( int f=last_cache_f ; f<end ; f++ )
    {
        last_cache_f++;
        TopSortTokens(frame_toks[f].toks, &token_list);

        for( size_t i=0 ; i<token_list.size() ; i++ )
        {
            if( token_list[i]!=NULL )
            {
                token_list[i]->state = cache_fst1.AddState();
            }
        }
    }

    cache_fst1.SetStart(0);// sets the start state
}

// outputs a list in topological order
void KdOnlineLDecoder::TopSortTokens(KdToken2 *tok_list,
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
    QString dbg_buf;
    int count = 0;
    for (KdToken2 *tok = tok_list; tok != NULL; tok = tok->next)
    {
        count++;
        dbg_buf += QString::number(tok->olabel);
        dbg_buf += "->";
        token2pos[tok] = num_toks - ++cur_pos;
    }
//    qDebug() << "TPS---->" << num_toks
//             << "u" << count;
//    qDebug() << dbg_buf;

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

// computes final-costs for the final frame. It outputs to final-costs, a map from the KdToken*
// pointer to the final-prob of the corresponding state, for all Tokens
// that correspond to states that have final-probs.
void KdOnlineLDecoder::ComputeFinalCosts(unordered_map<KdToken2 *, float> *final_costs,
        float *final_relative_cost, float *final_best_cost)
{
    if (final_costs != NULL)
        final_costs->clear();
    const Elem *final_toks = elements.GetList();
    float infinity = std::numeric_limits<float>::infinity();
    float best_cost = infinity,
            best_cost_with_final = infinity;

    QString dbg_buf;
    while (final_toks != NULL)
    {
        KdStateId state = final_toks->key;
        KdToken2 *tok = final_toks->val;
        dbg_buf += QString::number(state);
        dbg_buf += "->";
        float final_cost = fst_->Final(state).Value();
        float cost = tok->tot_cost,
                cost_with_final = cost + final_cost;
        best_cost = std::min(cost, best_cost);
        best_cost_with_final = std::min(cost_with_final, best_cost_with_final);
        if (final_costs != NULL && final_cost != infinity)
            (*final_costs)[tok] = final_cost;
        final_toks = final_toks->tail;
    }
    qDebug() << "ComputeFinalCosts---->" << best_cost;
    qDebug() << dbg_buf;
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

void KdOnlineLDecoder::MakeLattice(KdCompactLattice *ofst)
{
    KdLattice raw_fst;
    double lat_beam = config_.lattice_beam;
    getDiffTime(start_t);
    dbg_times += "S:";
    dbg_times += getLDiffTime();
    RawLattice(&raw_fst);
    dbg_times += " R:";
    dbg_times += getLDiffTime();

    kd_PruneLattice(lat_beam, &raw_fst);
    dbg_times += " P:";
    dbg_times += getLDiffTime();
    kd_detLatPhonePrunedW(trans_model_, &raw_fst,
                          lat_beam, ofst, config_.det_opts);
    dbg_times += " D:";
    dbg_times += getLDiffTime();
}

QVector<BtWord> KdOnlineLDecoder::getResult(KdCompactLattice *out_fst)
{
    MakeLattice(out_fst);
    if( out_fst->Start() )
    {
        return result;
    }

    KdMBR *mbr = NULL;
    mbr = new KdMBR(out_fst);
    result = mbr->getResult();

    HaveSilence();
    CalcFinal();
    return result;
}

void KdOnlineLDecoder::CalcFinal()
{
    int word_count = result.size();
    QString buf;
    for( int i=0 ; i<word_count ; i++ )
    {
        if( i==0 )
        {
            if( result[i].end<0.15 )
            {
                qDebug() << "$$$$$$$ skipped " << result[i].end
                         <<  result[i].word;
                continue;
            }
        }

        int f_end = result[i].end*100;
        if( (uframe-f_end)>BT_MIN_SIL )
        {
            result[i].is_final = 1;
        }
        buf += result[i].word;
        buf += "(";
        buf += QString::number(f_end);
        buf += ")";
        buf += " ";
    }

    if( word_count )
    {
        qDebug() << ">>> " << result.last().end
                 << uframe << buf << result.size();
    }
}

// Returns "true" if the current hypothesis ends with long silence
void KdOnlineLDecoder::HaveSilence()
{
    status.state = KD_STATE_NORMAL;
    int word_count = result.size();

    if( word_count )
    {
        float end_time = result.last().end;
        int diff = uframe - end_time*100;
        if( diff>9 )
        {
            status.max_frame = end_time*100;
            status.max_frame += status.min_frame;
            status.state = KD_STATE_SILENCE;
        }
    }
    else if( uframe>150 )
    {
        status.state = KD_STATE_NULL; //reset decoder
    }

    if( uframe>1000 ) //cut imidietly if more than 10s
    {
        status.state = KD_STATE_BLOWN; //reset decoder
        qDebug() << "We ARE BLOWN";
    }
}

int KdOnlineLDecoder::Decode()
{
    checkReset(); //check if need reset
    ProcessNonemitting(std::numeric_limits<float>::max());
    int frame_max = decodable->NumFramesReady();
//    for ( frame=0 ; frame<opts.batch_size ; frame++ )
    while( frame_num<frame_max )
    {
        if ( (uframe%config_.prune_interval)==0 )
        {
//            PruneActiveTokens(config_.lattice_beam * config_.prune_scale);
        }
        float weight_cutoff = ProcessEmitting();
        ProcessNonemitting(weight_cutoff);

        uframe++;
    }
}

void KdOnlineLDecoder::checkReset()
{
    dbg_times += " E:";
    dbg_times += getDiffTime(start_t);
//    qDebug() << "Check Reset" << dbg_times;
    if( status.state==KD_STATE_NULL ||
        status.state==KD_STATE_BLOWN  )
    {
        qDebug() << "----------Reset Null"
                 << status.max_frame << uframe;
        frame_num -= 35;
    }
    else if( status.state==KD_STATE_SILENCE )
    {
        int diff = frame_num - status.max_frame;
        qDebug() << "------------Reset Sil"
                 << status.max_frame << diff << uframe;
        frame_num -= diff;
    }

    if( status.state!=KD_STATE_NORMAL )
    {
        status.min_frame = frame_num;
        status.max_frame = 0;
        ResetDecoder(); // this reset uframe

        cache_fst1.DeleteStates();
        last_cache_f = 0;
    }
    start_t = clock();
    dbg_times = "";
}

//sample_rate: 16000 channel: 2 chunk_size: 57344
//Check Reset " E:15ms"
//Check Reset "S:0ms F:0ms C:30ms R:27ms P:68ms D:7ms E:193ms"
//>>>  1.93 194 "delta oscar mike " 3
//Check Reset "S:0ms F:0ms C:57ms R:50ms P:126ms D:11ms E:297ms"
//>>>  2.93 294 "delta oscar mike super one " 5
//Check Reset "S:0ms F:0ms C:87ms R:78ms P:193ms D:17ms E:436ms"
//>>>  3.43 394 "delta oscar mike super one november " 6
//Check Reset "S:0ms F:0ms C:108ms R:97ms P:243ms D:24ms E:523ms"
//------------Reset Sil 343 51 394
