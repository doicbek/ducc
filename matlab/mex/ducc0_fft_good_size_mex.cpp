/*
 * DUCC0 FFT Good Size MEX Interface
 * 
 * MATLAB MEX gateway for finding efficient FFT sizes
 * 
 * Usage:
 *   n_good = ducc0_fft_good_size_mex(n, real)
 * 
 * Inputs:
 *   n: Target length
 *   real: Whether to find good size for real FFT (optional, default: false)
 * 
 * Output:
 *   n_good: Efficient FFT size >= n
 */

#include "mex.h"
#include "ducc0/fft/fft1d_impl.h"
#include "ducc0/infra/error_handling.h"
#include <cstddef>

using namespace ducc0;
using namespace ducc0::detail_fft;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 1) {
            mexErrMsgIdAndTxt("DUCC0:FFT:GoodSize:InputError", 
                "At least one input (target length) required");
        }
        
        // Get target length
        double n_val = mxGetScalar(prhs[0]);
        if (n_val < 0 || n_val > static_cast<double>(numeric_limits<size_t>::max())) {
            mexErrMsgIdAndTxt("DUCC0:FFT:GoodSize:InputError", 
                "Invalid target length");
        }
        size_t n = static_cast<size_t>(n_val);
        
        // Get real flag
        bool real = false;
        if (nrhs > 1) {
            real = mxGetScalar(prhs[1]) != 0;
        }
        
        // Find good size
        size_t n_good;
        if (real) {
            n_good = util1d::good_size_real(n);
        } else {
            n_good = util1d::good_size_cmplx(n);
        }
        
        // Create output
        plhs[0] = mxCreateDoubleScalar(static_cast<double>(n_good));
        
    } catch (const exception &e) {
        mexErrMsgIdAndTxt("DUCC0:FFT:GoodSize:Error", "%s", e.what());
    }
}

