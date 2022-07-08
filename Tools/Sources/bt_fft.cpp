#include "bt_fft.h"
#include <QDebug>
#include <QtMath>
#define KD_M_2PI 6.283185307179586476925286766559005

KdFFT::KdFFT(int N): KdSrcFFT(N/2)
{
    fft_size = N;
}

// forward = true instead of inverse
void KdFFT::Compute(float *data)
{
    int N = fft_size;
    int N2 = N/2;

    KdSrcFFT::Compute(data, true, &temp_buffer_);

    float rootN_re, rootN_im;  // exp(-2pi/N)
    int forward_sign = -1;

    rootN_re = std::cos(KD_M_2PI/N *forward_sign);
    rootN_im = std::sin(KD_M_2PI/N *forward_sign);

    float kN_re = -forward_sign;
    float kN_im = 0.0;  // exp(-2pik/N)

    for( int k = 1; 2*k <= N2; k++ )
    {
        // complex mul (kN = rootN*kN)
        kN_re = (kN_re * rootN_re) - (kN_im * rootN_im);
        kN_im = (kN_re * rootN_im) + (kN_im * rootN_re);

        float Ck_re, Ck_im, Dk_re, Dk_im;
        // C_k = 1/2 (B_k + B_{N/2 - k}^*) :
        Ck_re = 0.5 * (data[2*k] + data[N - 2*k]);
        Ck_im = 0.5 * (data[2*k + 1] - data[N - 2*k + 1]);
        // re(D_k)= 1/2 (im(B_k) + im(B_{N/2-k})):
        Dk_re = 0.5 * (data[2*k + 1] + data[N - 2*k + 1]);
        // im(D_k) = -1/2 (re(B_k) - re(B_{N/2-k}))
        Dk_im =-0.5 * (data[2*k] - data[N - 2*k]);
        // A_k = C_k + 1^(k/N) D_k:
        data[2*k] = Ck_re;  // A_k <-- C_k
        data[2*k+1] = Ck_im;
        // now A_k += D_k 1^(k/N)
        data[2*k]   += Dk_re*kN_re - Dk_im*kN_im;//mul
        data[2*k+1] += Dk_re*kN_im + Dk_im*kN_re;//mul

        int kdash = N2 - k;
        if( kdash!=k )
        {
            // Next we handle the index k' = N/2 - k.
            data[2*kdash] = Ck_re;  // A_k' <-- C_k'
            data[2*kdash+1] = -Ck_im;
            // now A_k' += D_k' 1^(k'/N)
            // We use 1^(k'/N) = 1^((N/2 - k) / N) = 1^(1/2) 1^(-k/N) = -1 * (1^(k/N))^*
            // so it's the same as 1^(k/N) but with the float part negated.
            data[2*kdash]   += -Dk_re*kN_re + Dk_im*kN_im;//mul
            data[2*kdash+1] +=  Dk_re*kN_im + Dk_im*kN_re;//mul
        }
    }
    // k = 0.
    float zeroth = data[0] + data[1];
    float   n2th = data[0] - data[1];
    data[0] = zeroth;
    data[1] = n2th;
}

KdSrcFFT::KdSrcFFT(int N)
{
    if(  (N & (N-1))!=0 || N <= 1)
    {
        qDebug() << "KdSrcFFT called with invalid number of points "
                  << N;
    }
    N_ = N;
    logn_ = 0;
    while (N > 1)
    {
        N >>= 1;
        logn_ ++;
    }
    ComputeTables();
}

void KdSrcFFT::ComputeTables()
{
    int    imax, lg2, i, j;
    int     m, m2, m4, m8, nel, n;
    float    *cn, *spcn, *smcn, *c3n, *spc3n, *smc3n;
    float    ang, c, s;

    lg2 = logn_ >> 1;
    if( logn_ & 1) lg2++;
    brseed_ = new int[1 << lg2];
    brseed_[0] = 0;
    brseed_[1] = 1;
    for( j = 2; j <= lg2; j++)
    {
        imax = 1 << (j - 1);
        for( i = 0; i < imax; i++)
        {
            brseed_[i] <<= 1;
            brseed_[i + imax] = brseed_[i] + 1;
        }
    }

    if( logn_ < 4)
    {
        tab_ = NULL;
    }
    else
    {
        tab_ = new float* [logn_-3];
        for( i = logn_; i>=4 ; i--)
        {
            /* Compute a few constants */
            m = 1 << i; m2 = m / 2; m4 = m2 / 2; m8 = m4 /2;

            /* Allocate memory for tables */
            nel = m4 - 2;

            tab_[i-4] = new float[6*nel];

            /* Initialize pointers */
            cn = tab_[i-4]; spcn  = cn + nel;  smcn  = spcn + nel;
            c3n = smcn + nel;  spc3n = c3n + nel; smc3n = spc3n + nel;

            /* Compute tables */
            for( n = 1; n < m4; n++)
            {
                if( n==m8 )
                    continue;
                ang = n * KD_M_2PI / m;
                c = std::cos(ang); s = std::sin(ang);
                *cn++ = c; *spcn++ = - (s + c); *smcn++ = s - c;
                ang = 3 * n * KD_M_2PI / m;
                c = std::cos(ang); s = std::sin(ang);
                *c3n++ = c; *spc3n++ = - (s + c); *smc3n++ = s - c;
            }
        }
    }
}

KdSrcFFT::~KdSrcFFT()
{
    delete [] brseed_;
    if( tab_!=NULL)
    {
        for( int i = 0; i < logn_-3; i++)
            delete [] tab_[i];
        delete [] tab_;
    }
}

void KdSrcFFT::Compute(float *xr, float *xi, bool forward)
{
    if( !forward)
    {  // reverse float and imaginary parts for complex FFT.
        float *tmp = xr;
        xr = xi;
        xi = tmp;
    }
    ComputeRecursive(xr, xi, logn_);
    if( logn_ > 1)
    {
        BitReversePermute(xr, logn_);
        BitReversePermute(xi, logn_);
    }
}

void KdSrcFFT::Compute(float *x, bool forward,
                       std::vector<float> *temp_buffer)
{
//    KALDI_ASSERT(temp_buffer!=NULL);
    if( temp_buffer->size()!=N_)
        temp_buffer->resize(N_);
    float *temp_ptr = &((*temp_buffer)[0]);
    for( int i = 0; i<N_ ; i++ )
    {
        x[i] = x[i * 2];  // put the float part in the first half of x.
        temp_ptr[i] = x[i * 2 + 1];  // put the imaginary part in temp_buffer.
    }
    // copy the imaginary part back to the second half of x.
    memcpy(static_cast<void*>(x + N_),
           static_cast<void*>(temp_ptr),
           sizeof(float) * N_);

    Compute(x, x + N_, forward);
    // Now change the format back to interleaved.
    memcpy(static_cast<void*>(temp_ptr),
           static_cast<void*>(x + N_),
           sizeof(float) * N_);
    for( int i = N_-1; i > 0; i--)
    {  // don't include 0,
        // in case int is unsigned, the loop would not terminate.
        // Treat it as a special case.
        x[i*2] = x[i];
        x[i*2 + 1] = temp_ptr[i];
    }
    x[1] = temp_ptr[0];  // special case of i = 0.
}

void KdSrcFFT::BitReversePermute(float *x, int logn)
{
    int      i, j, lg2, n;
    int      off, fj, gno, *brp;
    float    tmp, *xp, *xq;

    lg2 = logn >> 1;
    n = 1 << lg2;
    if( logn & 1) lg2++;

    /* Unshuffling loop */
    for( off = 1; off < n; off++)
    {
        fj = n * brseed_[off]; i = off; j = fj;
        tmp = x[i]; x[i] = x[j]; x[j] = tmp;
        xp = &x[i];
        brp = &(brseed_[1]);
        for( gno = 1; gno < brseed_[off]; gno++)
        {
            xp += n;
            j = fj + *brp++;
            xq = x + j;
            tmp = *xp; *xp = *xq; *xq = tmp;
        }
    }
}

void KdSrcFFT::ComputeRecursive(float *xr, float *xi, int logn)
{
    int    m, m2, m4, m8, nel, n;
    float    *xr1, *xr2, *xi1, *xi2;
    float    *cn = nullptr, *spcn = nullptr, *smcn = nullptr, *c3n = nullptr,
            *spc3n = nullptr, *smc3n = nullptr;
    float    tmp1, tmp2;
    float   sqhalf = M_SQRT1_2;

    /* Check range of logn */
    if( logn < 0)
    {
        qDebug() << "Error: logn is out of bounds in SRFFT";
    }

    /* Compute trivial cases */
    if( logn < 3)
    {
        if( logn==2) {  /* length m = 4 */
            xr2  = xr + 2;
            xi2  = xi + 2;
            tmp1 = *xr + *xr2;
            *xr2 = *xr - *xr2;
            *xr  = tmp1;
            tmp1 = *xi + *xi2;
            *xi2 = *xi - *xi2;
            *xi  = tmp1;
            xr1  = xr + 1;
            xi1  = xi + 1;
            xr2++;
            xi2++;
            tmp1 = *xr1 + *xr2;
            *xr2 = *xr1 - *xr2;
            *xr1 = tmp1;
            tmp1 = *xi1 + *xi2;
            *xi2 = *xi1 - *xi2;
            *xi1 = tmp1;
            xr2  = xr + 1;
            xi2  = xi + 1;
            tmp1 = *xr + *xr2;
            *xr2 = *xr - *xr2;
            *xr  = tmp1;
            tmp1 = *xi + *xi2;
            *xi2 = *xi - *xi2;
            *xi  = tmp1;
            xr1  = xr + 2;
            xi1  = xi + 2;
            xr2  = xr + 3;
            xi2  = xi + 3;
            tmp1 = *xr1 + *xi2;
            tmp2 = *xi1 + *xr2;
            *xi1 = *xi1 - *xr2;
            *xr2 = *xr1 - *xi2;
            *xr1 = tmp1;
            *xi2 = tmp2;
            return;
        }
        else if( logn==1) {   /* length m = 2 */
            xr2  = xr + 1;
            xi2  = xi + 1;
            tmp1 = *xr + *xr2;
            *xr2 = *xr - *xr2;
            *xr  = tmp1;
            tmp1 = *xi + *xi2;
            *xi2 = *xi - *xi2;
            *xi  = tmp1;
            return;
        }
        else if( logn==0) return;   /* length m = 1 */
    }

    /* Compute a few constants */
    m = 1 << logn; m2 = m / 2; m4 = m2 / 2; m8 = m4 /2;


    /* Step 1 */
    xr1 = xr; xr2 = xr1 + m2;
    xi1 = xi; xi2 = xi1 + m2;
    for( n = 0; n < m2; n++)
    {
        tmp1 = *xr1 + *xr2;
        *xr2 = *xr1 - *xr2;
        xr2++;
        *xr1++ = tmp1;
        tmp2 = *xi1 + *xi2;
        *xi2 = *xi1 - *xi2;
        xi2++;
        *xi1++ = tmp2;
    }

    /* Step 2 */
    xr1 = xr + m2; xr2 = xr1 + m4;
    xi1 = xi + m2; xi2 = xi1 + m4;
    for( n = 0; n < m4; n++)
    {
        tmp1 = *xr1 + *xi2;
        tmp2 = *xi1 + *xr2;
        *xi1 = *xi1 - *xr2;
        xi1++;
        *xr2++ = *xr1 - *xi2;
        *xr1++ = tmp1;
        *xi2++ = tmp2;
        // xr1++; xr2++; xi1++; xi2++;
    }

    /* Steps 3 & 4 */
    xr1 = xr + m2; xr2 = xr1 + m4;
    xi1 = xi + m2; xi2 = xi1 + m4;
    if( logn >= 4)
    {
        nel = m4 - 2;
        cn  = tab_[logn-4]; spcn  = cn + nel;  smcn  = spcn + nel;
        c3n = smcn + nel;  spc3n = c3n + nel; smc3n = spc3n + nel;
    }
    xr1++; xr2++; xi1++; xi2++;
    // xr1++; xi1++;
    for( n = 1; n < m4; n++)
    {
        if( n==m8)
        {
            tmp1 =  sqhalf * (*xr1 + *xi1);
            *xi1 =  sqhalf * (*xi1 - *xr1);
            *xr1 =  tmp1;
            tmp2 =  sqhalf * (*xi2 - *xr2);
            *xi2 = -sqhalf * (*xr2 + *xi2);
            *xr2 =  tmp2;
        }
        else
        {
            tmp2 = *cn++ * (*xr1 + *xi1);
            tmp1 = *spcn++ * *xr1 + tmp2;
            *xr1 = *smcn++ * *xi1 + tmp2;
            *xi1 = tmp1;
            tmp2 = *c3n++ * (*xr2 + *xi2);
            tmp1 = *spc3n++ * *xr2 + tmp2;
            *xr2 = *smc3n++ * *xi2 + tmp2;
            *xi2 = tmp1;
        }
        xr1++; xr2++; xi1++; xi2++;
    }

    /* Call ssrec again with half DFT length */
    ComputeRecursive(xr, xi, logn-1);

    /* Call ssrec again twice with one quarter DFT length.
     Constants have to be recomputed, because they are static! */
    // m = 1 << logn; m2 = m / 2;
    ComputeRecursive(xr + m2, xi + m2, logn - 2);
    // m = 1 << logn;
    m4 = 3 * (m / 4);
    ComputeRecursive(xr + m4, xi + m4, logn - 2);
}
