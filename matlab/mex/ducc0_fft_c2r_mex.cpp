/*
 * DUCC0 FFT C2R MEX Interface
 * 
 * MATLAB MEX gateway for complex-to-real FFT
 * 
 * Usage:
 *   out = ducc0_fft_c2r_mex(in, axes, lastsize, forward, inorm, nthreads)
 * 
 * Inputs:
 *   in: Complex input array (double or single)
 *   axes: Axes to transform (optional, default: all axes)
 *   lastsize: Output size of last axis (optional, default: 2*n-2)
 *   forward: Forward transform flag (optional, default: false for inverse)
 *   inorm: Normalization (0=none, 1=1/sqrt(N), 2=1/N) (optional, default: 0)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   out: Real output array
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
            mexErrMsgIdAndTxt("DUCC0:FFT:C2R:InputError", 
                "At least one input (array) required");
        }
        
        const mxArray *in_arr = prhs[0];
        if (mxIsEmpty(in_arr)) {
            plhs[0] = mxDuplicateArray(in_arr);
            return;
        }
        
        if (!mxIsComplex(in_arr)) {
            mexErrMsgIdAndTxt("DUCC0:FFT:C2R:InputError", 
                "Input must be complex for c2r");
        }
        
        // Parse optional parameters
        const mxArray *axes_arr = (nrhs > 1) ? prhs[1] : nullptr;
        size_t lastsize = getOptionalParam<size_t>(nrhs > 2 ? prhs[2] : nullptr, 0);
        bool forward = getOptionalParam<bool>(nrhs > 3 ? prhs[3] : nullptr, false);
        int inorm = getOptionalParam<int>(nrhs > 4 ? prhs[4] : nullptr, 0);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 5 ? prhs[5] : nullptr, 0);
        
        // Get input dimensions and convert to row-major
        mwSize ndim = mxGetNumberOfDimensions(in_arr);
        const mwSize *dims = mxGetDimensions(in_arr);
        vector<size_t> shape_ducc = matlabDimsToVector(dims, ndim);
        
        // Convert axes
        vector<size_t> axes = convertAxes(axes_arr, shape_ducc.size());
        size_t last_axis = axes.back();
        
        // Determine output size for last axis
        if (lastsize == 0) {
            lastsize = 2 * shape_ducc[last_axis] - 1;
        }
        
        // Verify lastsize is valid
        if ((lastsize / 2) + 1 != shape_ducc[last_axis]) {
            mexErrMsgIdAndTxt("DUCC0:FFT:C2R:InputError", 
                "Invalid lastsize. Must satisfy (lastsize/2)+1 == input_size");
        }
        
        // Output shape: same as input except last axis is lastsize
        vector<size_t> out_shape_ducc = shape_ducc;
        out_shape_ducc[last_axis] = lastsize;
        
        // Convert output shape to MATLAB dimensions
        mwSize out_ndim = out_shape_ducc.size();
        mwSize *out_dims = new mwSize[out_ndim];
        vectorDimsToMatlab(out_shape_ducc, out_dims);
        
        // Determine data type
        mxClassID class_id = mxGetClassID(in_arr);
        
        // Create output array (real)
        mxArray *out_arr = mxCreateNumericArray(out_ndim, out_dims, class_id, mxREAL);
        delete[] out_dims;
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Complex double input -> real double output
            vector<complex<double>> in_buffer;
            vector<double> out_buffer;
            
            size_t in_nelem = 1;
            for (size_t s : shape_ducc) in_nelem *= s;
            
            size_t out_nelem = 1;
            for (size_t s : out_shape_ducc) out_nelem *= s;
            
            in_buffer.resize(in_nelem);
            out_buffer.resize(out_nelem);
            
            // Copy input from MATLAB to buffer
            copyMatlabToBuffer<complex<double>>(in_arr, in_buffer.data(), shape_ducc);
            
            // Create DUCC array views
            cfmav<complex<double>> in_view(in_buffer.data(), shape_ducc, );
            vfmav<double> out_view(out_buffer.data(), out_shape_ducc, );
            
            // Compute normalization factor (using output shape)
            double fct = computeNormFactor<double>(inorm, out_shape_ducc, axes);
            
            // Perform c2r FFT
            c2r(in_view, out_view, axes, forward, fct, nthreads);
            
            // Copy output from buffer to MATLAB (real data)
            // Manual copy since copyBufferToMatlab template doesn't handle real types well
            double *out_data = static_cast<double *>(mxGetData(out_arr));
            mwSize out_ndim = out_shape_ducc.size();
            const mwSize *out_dims = mxGetDimensions(out_arr);
            
            // Pre-compute MATLAB strides
            vector<size_t> strides_matlab(out_ndim);
            strides_matlab[0] = 1;
            for (mwSize i = 1; i < out_ndim; ++i) {
                strides_matlab[i] = strides_matlab[i-1] * out_dims[i-1];
            }
            
            // Copy with dimension reordering
            vector<size_t> indices(out_shape_ducc.size(), 0);
            size_t ndim = out_shape_ducc.size();
            size_t nelem = 1;
            for (size_t s : out_shape_ducc) nelem *= s;
            
            for (size_t i = 0; i < nelem; ++i) {
                size_t idx_matlab = matlabLinearIndex(indices.data(), strides_matlab.data(), out_ndim);
                out_data[idx_matlab] = out_buffer[i];
                if (i < nelem - 1) {
                    incrementIndices(indices.data(), out_shape_ducc.data(), ndim);
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Complex single input -> real single output
            vector<complex<float>> in_buffer;
            vector<float> out_buffer;
            
            size_t in_nelem = 1;
            for (size_t s : shape_ducc) in_nelem *= s;
            
            size_t out_nelem = 1;
            for (size_t s : out_shape_ducc) out_nelem *= s;
            
            in_buffer.resize(in_nelem);
            out_buffer.resize(out_nelem);
            
            // Copy input from MATLAB to buffer
            copyMatlabToBuffer<complex<float>>(in_arr, in_buffer.data(), shape_ducc);
            
            // Create DUCC array views
            cfmav<complex<float>> in_view(in_buffer.data(), shape_ducc, );
            vfmav<float> out_view(out_buffer.data(), out_shape_ducc, );
            
            // Compute normalization factor (using output shape)
            float fct = computeNormFactor<float>(inorm, out_shape_ducc, axes);
            
            // Perform c2r FFT
            c2r(in_view, out_view, axes, forward, fct, nthreads);
            
            // Copy output from buffer to MATLAB (real data)
            // Manual copy since copyBufferToMatlab template doesn't handle real types well
            float *out_data = static_cast<float *>(mxGetData(out_arr));
            mwSize out_ndim = out_shape_ducc.size();
            const mwSize *out_dims = mxGetDimensions(out_arr);
            
            // Pre-compute MATLAB strides
            vector<size_t> strides_matlab(out_ndim);
            strides_matlab[0] = 1;
            for (mwSize i = 1; i < out_ndim; ++i) {
                strides_matlab[i] = strides_matlab[i-1] * out_dims[i-1];
            }
            
            // Copy with dimension reordering
            vector<size_t> indices(out_shape_ducc.size(), 0);
            size_t ndim = out_shape_ducc.size();
            size_t nelem = 1;
            for (size_t s : out_shape_ducc) nelem *= s;
            
            for (size_t i = 0; i < nelem; ++i) {
                size_t idx_matlab = matlabLinearIndex(indices.data(), strides_matlab.data(), out_ndim);
                out_data[idx_matlab] = out_buffer[i];
                if (i < nelem - 1) {
                    incrementIndices(indices.data(), out_shape_ducc.data(), ndim);
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:C2R:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

