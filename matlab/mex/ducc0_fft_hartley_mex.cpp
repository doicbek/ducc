/*
 * DUCC0 FFT Hartley MEX Interface
 * 
 * MATLAB MEX gateway for separable Hartley transforms
 * 
 * Usage:
 *   out = ducc0_fft_hartley_mex(in, axes, inorm, nthreads)
 * 
 * Inputs:
 *   in: Real input array (double or single)
 *   axes: Axes to transform (optional, default: all axes)
 *   inorm: Normalization (0=none, 1=1/sqrt(N), 2=1/N) (optional, default: 0)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   out: Real output array (same shape as input)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/fft/fft.h"
#include "ducc0/infra/error_handling.h"
#include <vector>

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

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

// Helper to get optional parameter
template<typename T>
T getOptionalParam(const mxArray *arr, T default_value)
{
    if (arr == nullptr || mxIsEmpty(arr)) {
        return default_value;
    }
    if constexpr (is_same_v<T, bool>) {
        return mxGetScalar(arr) != 0;
    } else {
        return static_cast<T>(mxGetScalar(arr));
    }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 1) {
            mexErrMsgIdAndTxt("DUCC0:FFT:Hartley:InputError", 
                "At least one input (array) required");
        }
        
        const mxArray *in_arr = prhs[0];
        if (mxIsEmpty(in_arr)) {
            plhs[0] = mxDuplicateArray(in_arr);
            return;
        }
        
        if (mxIsComplex(in_arr)) {
            mexErrMsgIdAndTxt("DUCC0:FFT:Hartley:InputError", 
                "Input must be real for Hartley transform");
        }
        
        // Parse optional parameters
        const mxArray *axes_arr = (nrhs > 1) ? prhs[1] : nullptr;
        int inorm = getOptionalParam<int>(nrhs > 2 ? prhs[2] : nullptr, 0);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 3 ? prhs[3] : nullptr, 0);
        
        // Get input dimensions and convert to row-major
        mwSize ndim = mxGetNumberOfDimensions(in_arr);
        const mwSize *dims = mxGetDimensions(in_arr);
        vector<size_t> shape_ducc = matlabDimsToVector(dims, ndim);
        
        // Convert axes
        vector<size_t> axes = convertAxes(axes_arr, shape_ducc.size());
        
        // Determine data type
        mxClassID class_id = mxGetClassID(in_arr);
        
        // Create output array (same shape and type as input)
        mxArray *out_arr = mxCreateNumericArray(ndim, dims, class_id, mxREAL);
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Real double input -> real double output
            vector<double> in_buffer;
            vector<double> out_buffer;
            
            size_t nelem = 1;
            for (size_t s : shape_ducc) nelem *= s;
            
            in_buffer.resize(nelem);
            out_buffer.resize(nelem);
            
            // Copy input from MATLAB to buffer
            copyMatlabToBuffer<double>(in_arr, in_buffer.data(), shape_ducc);
            
            // Create DUCC array views
            cmav<double> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            vmav<double> out_view(out_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            
            // Compute normalization factor
            double fct = computeNormFactor<double>(inorm, shape_ducc, axes);
            
            // Perform separable Hartley transform
            r2r_separable_hartley(in_view, out_view, axes, fct, nthreads);
            
            // Copy output from buffer to MATLAB
            copyBufferToMatlab<double>(out_buffer.data(), out_arr, shape_ducc);
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Real single input -> real single output
            vector<float> in_buffer;
            vector<float> out_buffer;
            
            size_t nelem = 1;
            for (size_t s : shape_ducc) nelem *= s;
            
            in_buffer.resize(nelem);
            out_buffer.resize(nelem);
            
            // Copy input from MATLAB to buffer
            copyMatlabToBuffer<float>(in_arr, in_buffer.data(), shape_ducc);
            
            // Create DUCC array views
            cmav<float> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            vmav<float> out_view(out_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            
            // Compute normalization factor
            float fct = computeNormFactor<float>(inorm, shape_ducc, axes);
            
            // Perform separable Hartley transform
            r2r_separable_hartley(in_view, out_view, axes, fct, nthreads);
            
            // Copy output from buffer to MATLAB
            copyBufferToMatlab<float>(out_buffer.data(), out_arr, shape_ducc);
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:Hartley:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

