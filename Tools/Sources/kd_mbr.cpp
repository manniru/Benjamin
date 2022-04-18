#include "kd_mbr.h"
#include <QDebug>

using namespace kaldi;
KdMBR::KdMBR()
{
    lexicon = bt_parseLexicon(BT_WORDS_PATH);
}

void KdMBR::compute(KdCompactLattice *clat_in)
{
    checkLattice(clat_in);
    createTimes(clat_in);
    createMBRLat(clat_in);
    getBestWords(clat_in);

    L_ = 0.0; // Set current edit-distance to 0

    MbrDecode();
}

void KdMBR::MbrDecode()
{
    AddEpsBest();
    computeGamma();
    calculateConf();

    RemoveEps();
}

void KdMBR::calculateConf()
{
    best_conf.clear();

    for( int q=0 ; q<best_wid.size() ; q++ )
    {
        double w_conf = 0;
        for( int j=0 ; j<lat_gamma[q].size() ; j++ )
        {
            if( lat_gamma[q][j].wid==best_wid[q] )
            {
                w_conf = lat_gamma[q][j].conf;
            }
        }

        if( best_wid[q]!=0 )
        {
            best_conf.push_back(w_conf);
        }
    }
}

// from words
void KdMBR::RemoveEps()
{
    for( int i=0 ; i<best_wid.size() ; i++ )
    {
        if( best_wid[i]==0 )
        {
            best_wid.erase(best_wid.begin()+i);
            i--;
        }
    }
}

// add eps at the beginning, end and between words
void KdMBR::AddEpsBest()
{
    RemoveEps();
    best_wid.resize(1 + best_wid.size() * 2);
    int len = best_wid.size();
    for( int i=len/2-1 ; i>=0 ; i-- )
    {
        best_wid[i*2 + 1] = best_wid[i];
        best_wid[i*2 + 2] = 0;
    }
    best_wid[0] = 0;
}

void KdMBR::computeGamma()
{
    using std::map;

    int state_count = mlat.size() - 1; // = Number of words
    int word_len = best_wid.size();

    if( max_state<state_count )
    {
        max_state = state_count;
//        qDebug() << "st" << state_count << word_len;
    }

    Vector<double> alpha(state_count+1); // index (1...N)
    Matrix<double> alpha_dash(state_count+1, word_len+1); // index (1...N, 0...Q)
    Vector<double> alpha_dash_arc(word_len+1); // index 0...Q
    Matrix<double> beta_dash(state_count+1, word_len+1); // index (1...N, 0...Q)
    Vector<double> beta_dash_arc(word_len+1); // index 0...Q
    std::vector<char> arc_type(word_len+1); // integer in {1,2,3}; index 1...Q
    std::vector<map<int, double> > gamma_map(word_len+1); // temp. form of gamma.

//    if( L_!=0 && Ltmp>L_ )
//    {
//        KALDI_WARN << "Edit distance increased: " << Ltmp << " > "
//                   << L_;
//    }
    L_ = editDistance(state_count, word_len, alpha, alpha_dash, alpha_dash_arc);
    KALDI_VLOG(2) << "L = " << L_;
    // omit line 10: zero when initialized.
    beta_dash(state_count, word_len) = 1.0; // Line 11.
    for( int state=state_count ; state>=2 ; state-- ) // all states
    {
        for( size_t i=0 ; i<mlat[state].size() ; i++ ) // all arcs
        {
            const KdMBRArc &arc = mlat_arc[mlat[state][i]];
            int s_a = arc.start_node;
            int w_a = arc.word;
            float p_a = arc.loglike;
            alpha_dash_arc(0) = alpha_dash(arc.start_node, 0) + kd_l_distance(w_a, 0, true); // line 14.
            for( int q=1; q<=word_len ; q++ )
            {
                int w_id = best_wid[q-1];;
                double a1 = alpha_dash(arc.start_node, q-1) + kd_l_distance(w_a, w_id);
                double a2 = alpha_dash(arc.start_node, q) + kd_l_distance(w_a, 0, true);
                double a3 = alpha_dash_arc(q-1) + kd_l_distance(0, w_id);
                if( a1<=a2 )
                {
                    if( a1<=a3 )
                    {
                        arc_type[q] = 1;
                        alpha_dash_arc(q) = a1;
                    }
                    else
                    {
                        arc_type[q] = 3;
                        alpha_dash_arc(q) = a3;
                    }
                }
                else
                {
                    if( a2 <= a3)
                    {
                        arc_type[q] = 2;
                        alpha_dash_arc(q) = a2;
                    }
                    else
                    {
                        arc_type[q] = 3;
                        alpha_dash_arc(q) = a3;
                    }
                }
            }

            beta_dash_arc.SetZero(); // line 19.
            for (int q = word_len; q >= 1; q--)
            {
                // line 21:
                beta_dash_arc(q) += Exp(alpha(s_a) + p_a - alpha(state)) * beta_dash(state, q);
                switch (arc_type[q])
                { // lines 22 and 23:
                case 1:
                    beta_dash(s_a, q-1) += beta_dash_arc(q);
                    // next: gamma(q, w(a)) += beta_dash_arc(q)
                    addToGamma(w_a, beta_dash_arc(q), &(gamma_map[q]));
                    break;
                case 2:
                    beta_dash(s_a, q) += beta_dash_arc(q);
                    break;
                case 3:
                    beta_dash_arc(q-1) += beta_dash_arc(q);
                    // next: gamma(q, epsilon) += beta_dash_arc(q)
                    addToGamma(0, beta_dash_arc(q), &(gamma_map[q]));
                    break;
                default:
                    qDebug() << "Invalid b_arc value"; // error in code.
                }
            }
            beta_dash_arc(0) += Exp(alpha(s_a) + p_a - alpha(state)) * beta_dash(state, 0);
            beta_dash(s_a, 0) += beta_dash_arc(0); // line 26.
        }
    }
    beta_dash_arc.SetZero(); // line 29.
    for( int q=word_len ; q>=1 ; q-- )
    {
        beta_dash_arc(q) += beta_dash(1, q);
        beta_dash_arc(q-1) += beta_dash_arc(q);
        addToGamma(0, beta_dash_arc(q), &(gamma_map[q]));
    }

    // convert map to vec
    kd_convertToVec(&gamma_map, &lat_gamma, word_len);
}

QVector<BtWord> KdMBR::getResult()
{
    QVector<BtWord> result;

    for( int i = 0; i<best_wid.size() ; i++ )
    {
        if( i>(b_times.size()-2) )
        {
            qDebug() << "Error 141: MBR Times"
                     << b_times.size()
                     << "on word" << lexicon[best_wid[i]];
            break;
        }
        BtWord word_buf;
        word_buf.conf  = best_conf[i];
        word_buf.start = b_times[i]  /100.0;
        word_buf.end   = b_times[i+1]/100.0;
        word_buf.time  = (word_buf.start+word_buf.end)/2;
        word_buf.word  = lexicon[best_wid[i]];
        word_buf.is_final = 0;

        result.push_back(word_buf);
    }

    if( result.size() )
    {
        if(  result.size()!=b_times.size() )
        {
            result.last().end = b_times.last()/100.0;
        }
    }

    return result;
}

void KdMBR::checkLattice(KdCompactLattice *clat)
{
    kd_CreateSuperFinal(clat); // Add just one super-final state).

    kaldi::uint64 props  = clat->Properties(fst::kFstProperties, false);
    // Check Top sort
    if( !(props & fst::kTopSorted) )
    {
        bool success = fst::TopSort(clat);
        if( success==false )
        {
            qDebug() << "Cycles detected in lattice.";
        }
    }
}

void KdMBR::createTimes(KdCompactLattice *clat)
{
    b_times = kd_getTimes(*clat);
}

void KdMBR::getBestWords(KdCompactLattice *clat)
{
    fst::VectorFst<fst::StdArc> fst;
    KdLattice lat;
    std::vector<int> alignment, words;
    fst::VectorFst<fst::StdArc> fst_shortest_path;
    fst::TropicalWeight weight;

    kd_RemoveAlignmentsFromCompactLattice(clat); // will be more efficient
    kd_ConvertLattice(*clat, &lat); // convert KdCompactLattice to KdLattice.
    kd_ConvertLattice(lat, &fst); // convert to FST.
    fst::ShortestPath(fst, &fst_shortest_path); // take shortest path of FST.
    kd_GetLinearSymbolSequence(fst_shortest_path, &alignment, &words, &weight);
//    KALDI_ASSERT(alignment.empty()); // we removed the alignment.
    best_wid = words;
}

// convert clat to mlat (lattic with KdMBRArc) and write to pre
void KdMBR::createMBRLat(KdCompactLattice *clat)
{
    // convert clat" KdMBRArc
    int state_count = clat->NumStates();
    mlat.clear();
    mlat_arc.clear();
    mlat.resize(state_count+1);
    int arc_id = 0;

    for (int n=0 ; n<state_count ; n++ )
    {
        for (fst::ArcIterator<KdCompactLattice> aiter(*clat, n);
             !aiter.Done(); aiter.Next())
        {
            const KdCLatArc &carc = aiter.Value();
            KdMBRArc arc; // in our local format.
            arc.word = carc.ilabel; //==carc.olabel
            arc.start_node = n+1; // add 1 (indexed from 1)
            arc.end_node = carc.nextstate + 1; // convert to 1-based.
            arc.loglike = - (carc.weight.weight.g_cost*0.5 +
                             carc.weight.weight.a_cost*0.8);

            mlat[arc.end_node].push_back(arc_id);
            arc_id++;
            mlat_arc.push_back(arc);
        }
    }
}


double KdMBR::editDistance(int N, int Q,
                           Vector<double> &alpha,
                           Matrix<double> &alpha_dash,
                           Vector<double> &alpha_dash_arc)
{
    alpha(1) = 0.0; // = log(1).  Line 5.
    alpha_dash(1, 0) = 0.0; // Line 5.
    for (int q = 1; q <= Q; q++)
    {
        alpha_dash(1, q) = alpha_dash(1, q-1) + kd_l_distance(0, best_wid[q-1]); // Line 7.
    }
    for (int n = 2; n <= N; n++)
    {
        double alpha_n = kLogZeroDouble;
        for (size_t i = 0; i < mlat[n].size(); i++)
        {
            const KdMBRArc &arc = mlat_arc[mlat[n][i]];
            alpha_n = LogAdd(alpha_n, alpha(arc.start_node) + arc.loglike);
        }
        alpha(n) = alpha_n; // Line 10.
        // Line 11 omitted: matrix was initialized to zero.
        for (size_t i = 0; i < mlat[n].size(); i++)
        {
            KdMBRArc arc = mlat_arc[mlat[n][i]];
            int s_a = arc.start_node;

            // for q = 0
            alpha_dash_arc(0) = alpha_dash(s_a, 0) + kd_l_distance(arc.word, 0, true);
            alpha_dash(n, 0) += Exp(alpha(s_a) + arc.loglike - alpha(n)) * alpha_dash_arc(0);
            for( int q=1 ; q<=Q ; q++ )
            {
                int word_id = best_wid[q-1];
                double a1 = alpha_dash(s_a, q-1) + kd_l_distance(arc.word, word_id);
                double a2 = alpha_dash(s_a, q)   + kd_l_distance(arc.word, 0, true);
                double a3 = alpha_dash_arc(q-1)  + kd_l_distance(0, word_id);
                alpha_dash_arc(q) = std::min(a1, std::min(a2, a3));
                // line 19:
                alpha_dash(n, q) += Exp(alpha(s_a) + arc.loglike - alpha(n)) * alpha_dash_arc(q);
            }
        }
    }
    return alpha_dash(N, Q); // line 23.
}
