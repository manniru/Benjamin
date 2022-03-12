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

void KdOnlineLDecoder::RawLattice(KdLattice *ofst)
{
    float infinity = std::numeric_limits<float>::infinity();
    int end = frame_toks.size();
    createStates(ofst);
    dbg_times += " C:";
    dbg_times += getLDiffTime();

    // Now create all arcs.
    for( int f=0 ; f<end ; f++ )
    {
        for( KdToken2 *tok=frame_toks[f].tail ; tok!=NULL ; tok=tok->prev )
        {
            KdFLink *link;
            for ( link=tok->links; link!=NULL; link=link->next )
            {
                KdStateId nextstate = link->next_tok->m_state;

                float cost_offset = 0.0;
                if( link->ilabel!=0 ) // emitting
                {
                    KALDI_ASSERT(f >= 0 && f<cost_offsets.length() );
                    cost_offset = cost_offsets[f];
                }
                KdLatticeArc::Weight arc_w(link->graph_cost, link->acoustic_cost - cost_offset);
                KdLatticeArc arc(link->ilabel, link->olabel,arc_w, nextstate);
                ofst->AddArc(tok->m_state, arc);
            }
            if( f==end-1 ) //final frame
            {
                float final_cost = fst_->Final(tok->state).Value();
                if( final_cost==infinity )
                {
                    final_cost = 0;
                }
                ofst->SetFinal(tok->m_state, LatticeWeight(final_cost, 0));
            }
        }
    }
}

// outputs a list in topological order
void KdOnlineLDecoder::TopSortTokens(KdToken2 *tok_list,
                                     std::vector<KdToken2 *> *out)
{
    unordered_map<KdToken2*, int> token2pos;
    typedef typename unordered_map<KdToken2*, int>::iterator IterType;
    int num_toks = 0;
    for (KdToken2 *tok = tok_list; tok != NULL; tok = tok->prev)
        num_toks++;
    int cur_pos = 0;
    // We assign the tokens numbers num_toks - 1, ... , 2, 1, 0.
    // This is likely to be in closer to topological order than
    // if we had given them ascending order, because of the way
    // new tokens are put at the front of the list.
    for (KdToken2 *tok = tok_list; tok != NULL; tok = tok->prev)
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

void KdOnlineLDecoder::createStates(KdLattice *ofst)
{
    int end = frame_toks.size();
    ofst->DeleteStates();
    last_cache_f = 0;

    // First create all states.
    std::vector<KdToken2*> token_list;
    for( int f=last_cache_f ; f<end ; f++ )
    {
        last_cache_f++;
        TopSortTokens(frame_toks[f].tail, &token_list);

        for( size_t i=0 ; i<token_list.size() ; i++ )
        {
            if( token_list[i]!=NULL )
            {
                token_list[i]->m_state = ofst->AddState();
            }
        }
    }

    ofst->SetStart(0);// sets the start state
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
    qDebug() << "Check Reset" << dbg_times;
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
