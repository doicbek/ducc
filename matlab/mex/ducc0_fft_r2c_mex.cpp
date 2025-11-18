/*
 * DUCC0 FFT R2C MEX Interface
 * 
 * MATLAB MEX gateway for real-to-complex FFT
 * 
 * Usage:
 *   out = ducc0_fft_r2c_mex(in, axes, forward, inorm, nthreads)
 * 
 * Inputs:
 *   in: Real input array (double or single)
 *   axes: Axes to transform (optional, default: all axes)
 *   forward: Forward transform flag (optional, default: true)
 *   inorm: Normalization (0=none, 1=1/sqrt(N), 2=1/N) (optional, default: 0)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   out: Complex output array (same shape except last axis is n//2+1)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/fft/fft.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>

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

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 1) {
            mexErrMsgIdAndTxt("DUCC0:FFT:R2C:InputError", 
                "At least one input (array) required");
        }
        
        const mxArray *in_arr = prhs[0];
        if (mxIsEmpty(in_arr)) {
            plhs[0] = mxDuplicateArray(in_arr);
            return;
        }
        
        if (mxIsComplex(in_arr)) {
            mexErrMsgIdAndTxt("DUCC0:FFT:R2C:InputError", 
                "Input must be real for r2c");
        }
        
        // Parse optional parameters
        const mxArray *axes_arr = (nrhs > 1) ? prhs[1] : nullptr;
        bool forward = getOptionalParam<bool>(nrhs > 2 ? prhs[2] : nullptr, true);
        int inorm = getOptionalParam<int>(nrhs > 3 ? prhs[3] : nullptr, 0);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 4 ? prhs[4] : nullptr, 0);
        
        // Get input dimensions and convert to row-major
        mwSize ndim = mxGetNumberOfDimensions(in_arr);
        const mwSize *dims = mxGetDimensions(in_arr);
        vector<size_t> shape_ducc = matlabDimsToVector(dims, ndim);
        
        // Convert axes
        vector<size_t> axes = convertAxes(axes_arr, shape_ducc.size());
        
        // Determine data type
        mxClassID class_id = mxGetClassID(in_arr);
        
        // Output shape: same as input except last axis in axes is n//2+1
        vector<size_t> out_shape_ducc = shape_ducc;
        size_t last_axis = axes.back();
        out_shape_ducc[last_axis] = (shape_ducc[last_axis] / 2) + 1;
        
        // Convert output shape to MATLAB dimensions
        mwSize out_ndim = out_shape_ducc.size();
        mwSize *out_dims = new mwSize[out_ndim];
        vectorDimsToMatlab(out_shape_ducc, out_dims);
        
        // Create output array (complex)
        mxArray *out_arr = mxCreateNumericArray(out_ndim, out_dims, class_id, mxCOMPLEX);
        delete[] out_dims;
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Real double input -> complex double output
            vector<double> in_buffer;
            vector<complex<double>> out_buffer;
            
            size_t in_nelem = 1;
            for (size_t s : shape_ducc) in_nelem *= s;
            
            size_t out_nelem = 1;
            for (size_t s : out_shape_ducc) out_nelem *= s;
            
            in_buffer.resize(in_nelem);
            out_buffer.resize(out_nelem);
            
            // Copy input from MATLAB to buffer
            copyMatlabToBuffer<double>(in_arr, in_buffer.data(), shape_ducc);
            
            // Create DUCC array views
            cfmav<double> in_view(in_buffer.data(), shape_ducc);
            vfmav<complex<double>> out_view(out_buffer.data(), out_shape_ducc);
            
            // Compute normalization factor
            double fct = computeNormFactor<double>(inorm, shape_ducc, axes);
            
            // Perform r2c FFT
            r2c(in_view, out_view, axes, forward, fct, nthreads);
            
            // Copy output from buffer to MATLAB
            copyBufferToMatlab<complex<double>>(out_buffer.data(), out_arr, out_shape_ducc);
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Real single input -> complex single output
            vector<float> in_buffer;
            vector<complex<float>> out_buffer;
            
            size_t in_nelem = 1;
            for (size_t s : shape_ducc) in_nelem *= s;
            
            size_t out_nelem = 1;
            for (size_t s : out_shape_ducc) out_nelem *= s;
            
            in_buffer.resize(in_nelem);
            out_buffer.resize(out_nelem);
            
            // Copy input from MATLAB to buffer
            copyMatlabToBuffer<float>(in_arr, in_buffer.data(), shape_ducc);
            
            // Create DUCC array views
            cfmav<float> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            vfmav<complex<float>> out_view(out_buffer.data(), out_shape_ducc, vector<ptrdiff_t>());
            
            // Compute normalization factor
            float fct = computeNormFactor<float>(inorm, shape_ducc, axes);
            
            // Perform r2c FFT
            r2c(in_view, out_view, axes, forward, fct, nthreads);
            
            // Copy output from buffer to MATLAB
            copyBufferToMatlab<complex<float>>(out_buffer.data(), out_arr, out_shape_ducc);
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:R2C:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}
