/*
 * DUCC0 FFT MEX Interface
 * 
 * MATLAB MEX gateway for DUCC FFT functions
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/fft/fft.h"
#include "ducc0/fft/fft1d_impl.h"
#include "ducc0/fft/fftnd_impl.h"
#include "ducc0/infra/threading.cc"
#include "ducc0/infra/mav.cc"
#include <vector>
#include <complex>

using namespace ducc0;
using namespace ducc0_mex;

// Helper to compute normalization factor
template<typename T>
T computeNormFactor(int inorm, const vector<size_t> &shape, const vector<size_t> &axes)
{
    if (inorm == 0) return T(1);
    
    size_t N = 1;
    for (auto a : axes) {
        N *= shape[a];
    }
    
    if (inorm == 1) return T(1.0 / sqrt(double(N)));
    if (inorm == 2) return T(1.0 / double(N));
    
    MR_fail("Invalid normalization value (must be 0, 1, or 2)");
    return T(1);
}

// MEX gateway for c2c (complex-to-complex FFT)
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Parse inputs: a, axes, forward, inorm, nthreads
        if (nrhs < 1) {
            mexErrMsgIdAndTxt("DUCC0:FFT:InputError", 
                "At least one input (array) required");
        }
        
        const mxArray *a_arr = prhs[0];
        const mxArray *axes_arr = (nrhs > 1) ? prhs[1] : nullptr;
        bool forward = (nrhs > 2) ? mxGetScalar(prhs[2]) != 0 : true;
        int inorm = (nrhs > 3) ? (int)mxGetScalar(prhs[3]) : 0;
        size_t nthreads = (nrhs > 4) ? (size_t)mxGetScalar(prhs[4]) : 0;
        
        // Convert input array
        ArrayDescriptor in_desc = mxArrayToDescriptor(a_arr);
        vector<size_t> axes = getAxesFromInput(axes_arr, in_desc.ndim);
        
        // Create output descriptor (same shape and type as input)
        ArrayDescriptor out_desc = in_desc;
        mxArray *out_arr = mxCreateNumericArray(
            mxGetNumberOfDimensions(a_arr),
            mxGetDimensions(a_arr),
            mxGetClassID(a_arr),
            mxIsComplex(a_arr) ? mxCOMPLEX : mxREAL
        );
        out_desc.data = mxGetData(out_arr);
        if (mxIsComplex(a_arr)) {
            // For complex, we need to handle both real and imag parts
            // This is simplified - may need adjustment
        }
        
        // Determine data type and call appropriate function
        if (isTypecode<complex<double>>(in_desc.typecode)) {
            auto myin = in_desc.to_cfmav<false, complex<double>>();
            auto myout = out_desc.to_vfmav<false, complex<double>>();
            double fct = computeNormFactor<double>(inorm, 
                vector<size_t>(myin.shape().begin(), myin.shape().end()), axes);
            c2c(myin, myout, axes, forward, fct, nthreads);
        } else if (isTypecode<complex<float>>(in_desc.typecode)) {
            auto myin = in_desc.to_cfmav<false, complex<float>>();
            auto myout = out_desc.to_vfmav<false, complex<float>>();
            float fct = computeNormFactor<float>(inorm,
                vector<size_t>(myin.shape().begin(), myin.shape().end()), axes);
            c2c(myin, myout, axes, forward, fct, nthreads);
        } else if (isTypecode<double>(in_desc.typecode)) {
            // Real input - use r2c internally
            auto myin = in_desc.to_cfmav<false, double>();
            // Output will be complex
            // This needs special handling for r2c
            mexErrMsgIdAndTxt("DUCC0:FFT:NotImplemented", 
                "Real-to-complex via c2c not yet implemented. Use r2c instead.");
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:TypeError", 
                "Unsupported data type for c2c");
        }
        
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

