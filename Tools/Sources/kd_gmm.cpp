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

float KdGmm::LogLikelihood(kaldi::VectorBase<float> &data)
{
    loglikes = gconsts;
    if( data.Dim()!=Dim() )
    {
        qDebug() << "DiagGmm::LogLikelihoods, dimension "
                 << "mismatch " << data.Dim() << " vs. " << Dim();
    }
    Vector<float> data_sq;
    int len = data.Dim();
    data_sq.Resize(len);
    for( int i=0 ; i<len ; i++ )
    {
        data_sq(i) = data(i) * data(i);
    }

    // loglikes +=  means * inv(vars) * data.
    int width  = loglikes.length();
    int height = data.Dim();
    for( int i=0 ; i<width ; i++ )
    {
        for( int j=0 ; j<height ; j++ )
        {
            loglikes[i] += means_invvars_(i,j) * data(j);
        }
    }
    // loglikes += -0.5 * inv(vars) * data_sq.
    width  = loglikes.length();
    height = data_sq.Dim();
    for( int i=0 ; i<width ; i++ )
    {
        for( int j=0 ; j<height ; j++ )
        {
            loglikes[i] -= 0.5 * inv_vars_(i,j) * data_sq(j);
        }
    }

    return calcLogSum();
}

void KdGmm::Read(std::istream &is)
{
    std::string token;
    ReadToken(is, true, &token);
    // <DiagGMMBegin> is for compatibility. Will be deleted later
    if (token != "<DiagGMMBegin>" && token != "<DiagGMM>")
    {
        qDebug() << "Expected <DiagGMM>, got " << token.c_str();
    }
    ReadToken(is, true, &token);
    if (token == "<GCONSTS>")
    {  // The gconsts are optional.
        gconsts = kd_VectorRead(is);
        ExpectToken(is, true, "<WEIGHTS>");
    }
    else
    {
        if (token != "<WEIGHTS>")
        {
            qDebug() << "DiagGmm::Read, expected <WEIGHTS> or <GCONSTS>, got "
                     << token.c_str();
        }
    }
    weights_ = kd_VectorRead(is);
    ExpectToken(is, true, "<MEANS_INVVARS>");
    means_invvars_.Read(is, true);
    ExpectToken(is, true, "<INV_VARS>");
    inv_vars_.Read(is, true);
    ReadToken(is, true, &token);
    // <DiagGMMEnd> is for compatibility. Will be deleted later
    if (token != "<DiagGMMEnd>" && token != "</DiagGMM>")
    {
        KALDI_ERR << "Expected </DiagGMM>, got " << token;
    }

    ComputeGconsts();  // safer option than trusting the read gconsts
}

void KdGmm::ComputeGconsts()
{
    int num_mix = weights_.length();
    int dim = Dim();
    float offset = -0.5 * M_LOG_2PI * dim;  // constant term in gconst.
    int num_bad = 0;

    // Resize if Gaussians have been removed during Update()
    if( num_mix!=gconsts.length() )
    {
        gconsts.resize(num_mix);
    }

    for( int mix=0 ; mix<num_mix ; mix++ )
    {
        float gc = logf(weights_[mix]) + offset;  // May be -inf if weights == 0
        for (int d = 0; d < dim; d++)
        {
            gc += 0.5 * Log(inv_vars_(mix, d)) - 0.5 * means_invvars_(mix, d)
                    * means_invvars_(mix, d) / inv_vars_(mix, d);
        }
        // Change sign for logdet because var is inverted. Also, note that
        // mean_invvars(mix, d)*mean_invvars(mix, d)/inv_vars(mix, d) is the
        // mean-squared times inverse variance, since mean_invvars(mix, d) contains
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
    int len = loglikes.length();
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

// assume float
QVector<float> kd_VectorRead(std::istream &is)
{
    std::ostringstream specific_error;
    QVector<float> buf;

    char *my_token =  "FV";
    std::string token;
    ReadToken(is, true, &token);
    if( token!=my_token )
    {
        if( token.length()>20 )
        {
            token = token.substr(0, 17) + "...";
        }
        specific_error << ": Expected token " << my_token << ", got " << token;
    }
    int32 size;
    ReadBasicType(is, true, &size);  // throws on error.
    if( (MatrixIndexT)size!=buf.length() )
    {
        buf.resize(size);
    }
    if( size>0 )
    {
        for( int i=0 ; i<size ; i++ )
        {
            float f;
            is.read(reinterpret_cast<char*>(&f), sizeof(float));
            buf[i] = f;
        }
    }
    if( is.fail() )
    {
        specific_error << "Error reading vector data (binary mode); truncated "
                          "stream? (size = " << size << ")";
    }
    return buf;
}
