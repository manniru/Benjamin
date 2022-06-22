#include <algorithm>
#include <functional>
#include <limits>
#include <string>
#include <vector>
#include <QDebug>

#include "kd_gmm.h"

#define KD_FLT_EPSILON 1.19209290e-7f

using namespace kaldi;

// Gets likelihood of data given this.
KdGmm::KdGmm()
{
    valid_gconsts_ = false;
}

float KdGmm::LogLikelihood(double *data, int len)
{
    loglikes = gconsts;

    int width  = loglikes.size();
    int height = len;
    for( int i=0 ; i<width ; i++ )
    {
        for( int j=0 ; j<height ; j++ )
        {
            loglikes[i] += means_invvars_.d[i][j] * data[j];
            loglikes[i] -= 0.5 * inv_vars.d[i][j] * data[j] * data[j];
        }
    }

    return calcLogSum();
}

void KdGmm::Read(std::istream &is)
{
    std::string token;
    ReadToken(is, true, &token);
    // <DiagGMMBegin> is for compatibility. Will be deleted later
    if( token!="<DiagGMMBegin>" && token!="<DiagGMM>" )
    {
        qDebug() << "Expected <DiagGMM>, got " << token.c_str();
    }
    ReadToken(is, true, &token);
    if( token=="<GCONSTS>" )
    {  // The gconsts are optional.
        gconsts = kd_VectorRead(is);
        ExpectToken(is, true, "<WEIGHTS>");
    }
    else
    {
        if( token!="<WEIGHTS>" )
        {
            qDebug() << "DiagGmm::Read, expected <WEIGHTS> or <GCONSTS>, got "
                     << token.c_str();
        }
    }
    weights_ = kd_VectorRead(is);
    ExpectToken(is, true, "<MEANS_INVVARS>");
    means_invvars_ = kd_MatrixRead(is);
    ExpectToken(is, true, "<INV_VARS>");
    inv_vars = kd_MatrixRead(is);
    ReadToken(is, true, &token);
    // <DiagGMMEnd> is for compatibility. Will be deleted later
    if( token!="<DiagGMMEnd>" && token!="</DiagGMM>" )
    {
        qDebug() << "Expected </DiagGMM>, got " << token.c_str();
    }

    ComputeGconsts();  // safer option than trusting the read gconsts
    if( BT_DELTA_SIZE!=means_invvars_.cols )
    {
        qDebug() << "DiagGmm::LogLikelihoods, dimension "
                 << "mismatch " << BT_DELTA_SIZE
                 << " vs. " << means_invvars_.cols;
    }
}

void KdGmm::ComputeGconsts()
{
    int num_mix = weights_.size();
    int dim = BT_DELTA_SIZE;
    float offset = -0.5 * M_LOG_2PI * dim;  // constant term in gconst.
    int num_bad = 0;

    // Resize if Gaussians have been removed during Update()
    if( num_mix!=gconsts.size() )
    {
        gconsts.resize(num_mix);
    }

    for( int mix=0 ; mix<num_mix ; mix++ )
    {
        float gc = logf(weights_[mix]) + offset;  // May be -inf if weights == 0
        for (int d = 0; d < dim; d++)
        {
            gc += 0.5 * Log(inv_vars.d[mix][d]) - 0.5 * means_invvars_.d[mix][d]
                    * means_invvars_.d[mix][d] / inv_vars.d[mix][d];
        }
        // Change sign for logdet because var is inverted. Also, note that
        // mean_invvars.d[mix][d]*mean_invvars.d[mix][d]/inv_vars.d[mix][d] is the
        // mean-squared times inverse variance, since mean_invvars.d[mix][d] contains
        // the mean times inverse variance.
        // So gc is the likelihood at zero feature value.

        if (std::isinf(gc))
        {
            num_bad++;
            // If positive infinity, make it negative infinity.
            // Want to make sure the answer becomes -inf in the end, not NaN.
            if (gc > 0)
            {
                gc = -gc;
            }
        }
        gconsts[mix] = gc;
    }

    valid_gconsts_ = true;
}

float KdGmm::calcLogSum()
{
    int len = loglikes.size();
    float max_elem = 0;

    ////////////////find max
    if( len )
    {
        max_elem = loglikes[0];
    }
    for( int i=0 ; i<len ; i++ )
    {
        if( max_elem<loglikes[i] )
        {
            max_elem = loglikes[i];
        }
    }
    /////////////////////end

    float cutoff = max_elem + logf(KD_FLT_EPSILON);

    double sum_relto_max_elem = 0.0;

    for( int i=0; i<len ; i++ )
    {
        if( loglikes[i]>=cutoff )
        {
            sum_relto_max_elem += expf(loglikes[i] - max_elem);
        }
    }
    return max_elem + logf(sum_relto_max_elem);
}
