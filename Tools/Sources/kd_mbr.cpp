#include "kd_mbr.h"
#include <QDebug>

using namespace kaldi;

struct GammaCompare
{
    // should be like operator <.  But we want reverse order
    // on the 2nd element (posterior), so it'll be like operator
    // > that looks first at the posterior.
    bool operator () (const std::pair<int, float> &a,
                      const std::pair<int, float> &b) const {
        if( a.second > b.second) return true;
        else if( a.second < b.second) return false;
        else return a.first > b.first;
    }
};

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
    QVector<QString> list[30];
    for( size_t counter=0 ; ; counter++ )
    {
        AddEpsBest();
        computeGamma();
        double delta_Q = 0.0; // change in objective function.

        one_best_conf.clear();

        for( int q=0 ; q<one_best_id.size() ; q++ )
        {// This loop updates one_best_id
            // gamma_[i] is sorted in reverse order so most likely one is first.
            const std::vector<std::pair<int, float> > &this_gamma = gamma_[q];
            double old_gamma = 0;
            double new_gamma = this_gamma[0].second;
            int old_word = one_best_id[q];
            int new_word = this_gamma[0].first; // new_word: new word
            for( int j=0 ; j<this_gamma.size() ; j++ )
            {
//                list[q].push_back(lexicon[this_gamma[j].first]);
                if( this_gamma[j].first==old_word )
                {
                    old_gamma = this_gamma[j].second;
                }
            }
            delta_Q += (old_gamma - new_gamma); // will be 0 or negative; a bound on
            // change in error.
            if( old_word!=new_word )
            {
                KALDI_VLOG(2) << "Changing word " << old_word << " to " << new_word;
            }
            one_best_id[q] = new_word;
            // build the outputs (time, confidences),
            if( one_best_id[q]!=0 )
            {
                float confidence = 0.0;
                for( int j=0 ; j<gamma_[q].size(); j++ )
                {
                    if( gamma_[q][j].first==one_best_id[q])
                    {
                        confidence = gamma_[q][j].second;
                        break;
                    }
                }
                one_best_conf.push_back(confidence);
            }
        }
        KALDI_VLOG(2) << "Iter = " << counter << ", delta-Q = " << delta_Q;
        if( delta_Q==0 )
            break;
        if( counter > 100)
        {
            KALDI_WARN << "Iterating too many times in MbrDecode; stopping.";
            break;
        }
    }

    RemoveEps();
}

// from words
void KdMBR::RemoveEps()
{
    for( int i=0 ; i<one_best_id.size() ; i++ )
    {
        if( one_best_id[i]==0 )
        {
            one_best_id.erase(one_best_id.begin()+i);
            i--;
        }
    }
}

// add eps at the beginning, end and between words
void KdMBR::AddEpsBest()
{
    RemoveEps();
    one_best_id.resize(1 + one_best_id.size() * 2);
    int len = one_best_id.size();
    for( int i=len/2-1 ; i>=0 ; i-- )
    {
        one_best_id[i*2 + 1] = one_best_id[i];
        one_best_id[i*2 + 2] = 0;
    }
    one_best_id[0] = 0;
}

// Figure 4 of paper; called from AccStats
double KdMBR::EditDistance(int N, int Q,
                           Vector<double> &alpha,
                           Matrix<double> &alpha_dash,
                           Vector<double> &alpha_dash_arc)
{
    alpha(1) = 0.0; // = log(1).  Line 5.
    alpha_dash(1, 0) = 0.0; // Line 5.
    for (int q = 1; q <= Q; q++)
    {
        alpha_dash(1, q) = alpha_dash(1, q-1) + l_distance(0, one_best_id[q-1]); // Line 7.
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
            float p_a = arc.loglike;

            // for q = 0
            alpha_dash_arc(0) = alpha_dash(s_a, 0) + l_distance(arc.word, 0, true);
            alpha_dash(n, 0) += Exp(alpha(s_a) + p_a - alpha(n)) * alpha_dash_arc(0);
            for( int q=1 ; q<=Q ; q++ )
            {
                int word_id = one_best_id[q-1];
                double a1 = alpha_dash(s_a, q-1) + l_distance(arc.word, word_id);
                double a2 = alpha_dash(s_a, q)   + l_distance(arc.word, 0, true);
                double a3 = alpha_dash_arc(q-1)  + l_distance(0, word_id);
                alpha_dash_arc(q) = std::min(a1, std::min(a2, a3));
                // line 19:
                alpha_dash(n, q) += Exp(alpha(s_a) + p_a - alpha(n)) * alpha_dash_arc(q);
            }
        }
    }
    return alpha_dash(N, Q); // line 23.
}

void KdMBR::computeGamma()
{
    using std::map;

    int state_count = mlat.size() - 1; // = Number of words
    int word_len = one_best_id.size();

    if( max_state<state_count )
    {
        max_state = state_count;
        qDebug() << "st" << state_count << word_len;
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
    L_ = EditDistance(state_count, word_len, alpha, alpha_dash, alpha_dash_arc);
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
            alpha_dash_arc(0) = alpha_dash(arc.start_node, 0) + l_distance(w_a, 0, true); // line 14.
            for( int q=1; q<=word_len ; q++ )
            {
                int w_id = one_best_id[q-1];;
                double a1 = alpha_dash(arc.start_node, q-1) + l_distance(w_a, w_id);
                double a2 = alpha_dash(arc.start_node, q) + l_distance(w_a, 0, true);
                double a3 = alpha_dash_arc(q-1) + l_distance(0, w_id);
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
                    AddToMap(w_a, beta_dash_arc(q), &(gamma_map[q]));
                    break;
                case 2:
                    beta_dash(s_a, q) += beta_dash_arc(q);
                    break;
                case 3:
                    beta_dash_arc(q-1) += beta_dash_arc(q);
                    // next: gamma(q, epsilon) += beta_dash_arc(q)
                    AddToMap(0, beta_dash_arc(q), &(gamma_map[q]));
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
        AddToMap(0, beta_dash_arc(q), &(gamma_map[q]));
    }
    for(int q = 1; q <= word_len; q++ )
    { // a check (line 35)
        double sum = 0.0;
        for (map<int, double>::iterator iter = gamma_map[q].begin();
             iter!=gamma_map[q].end(); ++iter) sum += iter->second;
        if( fabs(sum - 1.0) > 0.1)
            KALDI_WARN << "sum of gamma[" << q << ",s] is " << sum;
    }
    // The next part is where we take gamma, and convert
    // to the class member gamma_, which is using a different
    // data structure and indexed from zero, not one.
    gamma_.clear();
    gamma_.resize(word_len);
    for (int q = 1; q <= word_len; q++)
    {
        for (map<int, double>::iterator iter = gamma_map[q].begin();
             iter!=gamma_map[q].end(); ++iter)
            gamma_[q-1].push_back(
                        std::make_pair(iter->first, static_cast<float>(iter->second)));
        // sort gamma_[q-1] from largest to smallest posterior.
        GammaCompare comp;
        std::sort(gamma_[q-1].begin(), gamma_[q-1].end(), comp);
    }
}

void KdMBR::AddToMap(int i, double d, std::map<int, double> *gamma)
{
    if( d==0 )
        return;
    std::pair<const int, double> pr(i, d);
    std::pair<std::map<int, double>::iterator, bool> ret = gamma->insert(pr);
    if( !ret.second) // not inserted, so add to contents.
        ret.first->second += d;
}

double KdMBR::l_distance(int a, int b, bool penalize)
{
    if( a==b)
    {
        return 0.0;
    }
    else if( penalize )
    {
        return 1.0 + KD_MBR_DELTA;
    }
    else
    {
        return 1.0;
    }
}

QVector<BtWord> KdMBR::getResult()
{
    QVector<BtWord> result;
    std::vector<float> conf = one_best_conf;
    std::vector<int> words = one_best_id;

    for( int i = 0; i<words.size() ; i++ )
    {
        if( i>(b_times.size()-2) )
        {
            qDebug() << "Error 141: MBR Times"
                     << b_times.size()
                     << "on word" << lexicon[words[i]];
            break;
        }
        BtWord word_buf;
        word_buf.conf  = conf[i];
        word_buf.start = b_times[i]  /100.0;
        word_buf.end   = b_times[i+1]/100.0;
        word_buf.time  = (word_buf.start+word_buf.end)/2;
        word_buf.word  = lexicon[words[i]];
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
    one_best_id = words;
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
