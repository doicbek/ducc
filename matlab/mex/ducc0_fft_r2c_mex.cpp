/*
 * DUCC0 FFT R2C MEX Interface
 * 
 * MATLAB MEX gateway for real-to-complex FFT
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
    MR_fail("Invalid normalization value");
    return T(1);
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        if (nrhs < 1) {
            mexErrMsgIdAndTxt("DUCC0:FFT:InputError", 
                "At least one input (array) required");
        }
        
        const mxArray *a_arr = prhs[0];
        if (mxIsComplex(a_arr)) {
            mexErrMsgIdAndTxt("DUCC0:FFT:InputError", 
                "Input must be real for r2c");
        }
        
        const mxArray *axes_arr = (nrhs > 1) ? prhs[1] : nullptr;
        bool forward = (nrhs > 2) ? mxGetScalar(prhs[2]) != 0 : true;
        int inorm = (nrhs > 3) ? (int)mxGetScalar(prhs[3]) : 0;
        size_t nthreads = (nrhs > 4) ? (size_t)mxGetScalar(prhs[4]) : 0;
        
        ArrayDescriptor in_desc = mxArrayToDescriptor(a_arr);
        vector<size_t> axes = getAxesFromInput(axes_arr, in_desc.ndim);
        
        // Output shape: same as input except last axis is n//2+1
        mwSize ndim = mxGetNumberOfDimensions(a_arr);
        const mwSize *in_dims = mxGetDimensions(a_arr);
        mwSize *out_dims = new mwSize[ndim];
        for (mwSize i = 0; i < ndim; ++i) {
            out_dims[i] = in_dims[i];
        }
        // Last axis in axes (which is first in MATLAB order) gets reduced
        size_t last_axis_matlab = ndim - 1 - axes.back();
        out_dims[last_axis_matlab] = (out_dims[last_axis_matlab] / 2) + 1;
        
        mxClassID out_class = (mxGetClassID(a_arr) == mxSINGLE_CLASS) ? 
            mxSINGLE_CLASS : mxDOUBLE_CLASS;
        mxArray *out_arr = mxCreateNumericArray(ndim, out_dims, out_class, mxCOMPLEX);
        
        ArrayDescriptor out_desc = mxArrayToDescriptor(out_arr);
        
        if (isTypecode<double>(in_desc.typecode)) {
            auto myin = in_desc.to_cfmav<false, double>();
            auto myout = out_desc.to_vfmav<false, complex<double>>();
            double fct = computeNormFactor<double>(
                vector<size_t>(myin.shape().begin(), myin.shape().end()), axes);
            r2c(myin, myout, axes, forward, fct, nthreads);
        } else if (isTypecode<float>(in_desc.typecode)) {
            auto myin = in_desc.to_cfmav<false, float>();
            auto myout = out_desc.to_vfmav<false, complex<float>>();
            float fct = computeNormFactor<float>(
                vector<size_t>(myin.shape().begin(), myin.shape().end()), axes);
            r2c(myin, myout, axes, forward, fct, nthreads);
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:TypeError", 
                "Unsupported data type for r2c");
        }
        
        delete[] out_dims;
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

