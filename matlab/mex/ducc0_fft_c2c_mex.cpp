/*
 * DUCC0 FFT C2C MEX Interface
 * 
 * MATLAB MEX gateway for complex-to-complex FFT
 * 
 * Usage:
 *   out = ducc0_fft_c2c_mex(in, axes, forward, inorm, nthreads)
 * 
 * Inputs:
 *   in: Input array (real or complex)
 *   axes: Axes to transform (optional, default: all axes, 1-based)
 *   forward: Forward transform flag (optional, default: true)
 *   inorm: Normalization type (optional, default: 0)
 *         0: no normalization
 *         1: divide by sqrt(N)
 *         2: divide by N
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   out: Transformed array (same shape and type as input)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/fft/fft.h"
#include <vector>
#include <complex>
#include <memory>

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
            mexErrMsgIdAndTxt("DUCC0:FFT:C2C:InputError", 
                "At least one input (array) required");
        }
        
        const mxArray *in_arr = prhs[0];
        if (mxIsEmpty(in_arr)) {
            plhs[0] = mxDuplicateArray(in_arr);
            return;
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
        bool is_complex_in = mxIsComplex(in_arr);
        bool is_complex_out = true; // c2c always outputs complex
        
        // Create output array (same shape as input)
        mxArray *out_arr = mxCreateNumericArray(ndim, dims, class_id, mxCOMPLEX);
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            if (is_complex_in) {
                // Complex double input
                vector<complex<double>> in_buffer;
                vector<complex<double>> out_buffer;
                size_t nelem = 1;
                for (size_t s : shape_ducc) nelem *= s;
                
                in_buffer.resize(nelem);
                out_buffer.resize(nelem);
                
                // Copy input from MATLAB to buffer
                copyMatlabToBuffer<complex<double>>(in_arr, in_buffer.data(), shape_ducc);
                
                // Create DUCC array views
                cfmav<complex<double>> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
                vfmav<complex<double>> out_view(out_buffer.data(), shape_ducc, vector<ptrdiff_t>());
                
                // Compute normalization factor
                double fct = computeNormFactor<double>(inorm, shape_ducc, axes);
                
                // Perform FFT
                c2c(in_view, out_view, axes, forward, fct, nthreads);
                
                // Copy output from buffer to MATLAB
                copyBufferToMatlab<complex<double>>(out_buffer.data(), out_arr, shape_ducc);
            } else {
                // Real double input - use r2c internally
                vector<double> in_buffer;
                vector<complex<double>> out_buffer;
                size_t nelem = 1;
                for (size_t s : shape_ducc) nelem *= s;
                
                in_buffer.resize(nelem);
                
                // Copy input from MATLAB to buffer
                copyMatlabToBuffer<double>(in_arr, in_buffer.data(), shape_ducc);
                
                // Create DUCC array views
                cfmav<double> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
                
                // For r2c, output shape is different along last axis
                vector<size_t> out_shape = shape_ducc;
                out_shape[axes.back()] = (out_shape[axes.back()] / 2) + 1;
                size_t out_nelem = 1;
                for (size_t s : out_shape) out_nelem *= s;
                out_buffer.resize(out_nelem);
                
                vfmav<complex<double>> out_view(out_buffer.data(), out_shape, vector<ptrdiff_t>());
                
                // Compute normalization factor
                double fct = computeNormFactor<double>(inorm, shape_ducc, axes);
                
                // Perform r2c FFT
                r2c(in_view, out_view, axes, forward, fct, nthreads);
                
                // Update output array dimensions
                mwSize *out_dims = new mwSize[ndim];
                vectorDimsToMatlab(out_shape, out_dims);
                mxDestroyArray(out_arr);
                out_arr = mxCreateNumericArray(ndim, out_dims, class_id, mxCOMPLEX);
                delete[] out_dims;
                
                // Copy output from buffer to MATLAB
                copyBufferToMatlab<complex<double>>(out_buffer.data(), out_arr, out_shape);
            }
        } else if (class_id == mxSINGLE_CLASS) {
            if (is_complex_in) {
                // Complex single input
                vector<complex<float>> in_buffer;
                vector<complex<float>> out_buffer;
                size_t nelem = 1;
                for (size_t s : shape_ducc) nelem *= s;
                
                in_buffer.resize(nelem);
                out_buffer.resize(nelem);
                
                // Copy input from MATLAB to buffer
                copyMatlabToBuffer<complex<float>>(in_arr, in_buffer.data(), shape_ducc);
                
                // Create DUCC array views
                cfmav<complex<float>> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
                vfmav<complex<float>> out_view(out_buffer.data(), shape_ducc, vector<ptrdiff_t>());
                
                // Compute normalization factor
                float fct = computeNormFactor<float>(inorm, shape_ducc, axes);
                
                // Perform FFT
                c2c(in_view, out_view, axes, forward, fct, nthreads);
                
                // Copy output from buffer to MATLAB
                copyBufferToMatlab<complex<float>>(out_buffer.data(), out_arr, shape_ducc);
            } else {
                // Real single input - use r2c internally
                vector<float> in_buffer;
                vector<complex<float>> out_buffer;
                size_t nelem = 1;
                for (size_t s : shape_ducc) nelem *= s;
                
                in_buffer.resize(nelem);
                
                // Copy input from MATLAB to buffer
                copyMatlabToBuffer<float>(in_arr, in_buffer.data(), shape_ducc);
                
                // Create DUCC array views
                cfmav<float> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
                
                // For r2c, output shape is different along last axis
                vector<size_t> out_shape = shape_ducc;
                out_shape[axes.back()] = (out_shape[axes.back()] / 2) + 1;
                size_t out_nelem = 1;
                for (size_t s : out_shape) out_nelem *= s;
                out_buffer.resize(out_nelem);
                
                vfmav<complex<float>> out_view(out_buffer.data(), out_shape, vector<ptrdiff_t>());
                
                // Compute normalization factor
                float fct = computeNormFactor<float>(inorm, shape_ducc, axes);
                
                // Perform r2c FFT
                r2c(in_view, out_view, axes, forward, fct, nthreads);
                
                // Update output array dimensions
                mwSize *out_dims = new mwSize[ndim];
                vectorDimsToMatlab(out_shape, out_dims);
                mxDestroyArray(out_arr);
                out_arr = mxCreateNumericArray(ndim, out_dims, class_id, mxCOMPLEX);
                delete[] out_dims;
                
                // Copy output from buffer to MATLAB
                copyBufferToMatlab<complex<float>>(out_buffer.data(), out_arr, out_shape);
            }
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:C2C:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}



