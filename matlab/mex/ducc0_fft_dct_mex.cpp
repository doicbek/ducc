/*
 * DUCC0 FFT DCT MEX Interface
 * 
 * MATLAB MEX gateway for discrete cosine transforms
 * 
 * Usage:
 *   out = ducc0_fft_dct_mex(in, type, axes, inorm, nthreads)
 * 
 * Inputs:
 *   in: Real input array (double or single)
 *   type: DCT type (1, 2, 3, or 4) (required)
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
T computeNormFactor(int inorm, const vector<size_t> &shape, const vector<size_t> &axes, int type)
{
    if (inorm == 0) return T(1);
    
    size_t N = 1;
    for (auto a : axes) {
        N *= shape[a];
    }
    
    // For DCT, normalization uses factor 2 for all types except type 1
    if (type == 1) {
        // Type 1: factor 2*(N-1)
        N = 2 * (N - 1);
    } else {
        // Types 2, 3, 4: factor 2*N
        N = 2 * N;
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
        if (nrhs < 2) {
            mexErrMsgIdAndTxt("DUCC0:FFT:DCT:InputError", 
                "At least two inputs required: array and type");
        }
        
        const mxArray *in_arr = prhs[0];
        if (mxIsEmpty(in_arr)) {
            plhs[0] = mxDuplicateArray(in_arr);
            return;
        }
        
        if (mxIsComplex(in_arr)) {
            mexErrMsgIdAndTxt("DUCC0:FFT:DCT:InputError", 
                "Input must be real for DCT");
        }
        
        // Parse required and optional parameters
        int type = (int)mxGetScalar(prhs[1]);
        if (type < 1 || type > 4) {
            mexErrMsgIdAndTxt("DUCC0:FFT:DCT:InputError", 
                "DCT type must be 1, 2, 3, or 4");
        }
        
        const mxArray *axes_arr = (nrhs > 2) ? prhs[2] : nullptr;
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
            cfmav<double> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            vfmav<double> out_view(out_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            
            // Compute normalization factor
            double fct = computeNormFactor<double>(inorm, shape_ducc, axes, type);
            bool ortho = (inorm == 1);
            
            // Perform DCT
            dct(in_view, out_view, axes, type, fct, ortho, nthreads);
            
            // Copy output from buffer to MATLAB (real data)
            double *out_data = static_cast<double *>(mxGetData(out_arr));
            mwSize out_ndim = shape_ducc.size();
            const mwSize *out_dims = mxGetDimensions(out_arr);
            
            // Pre-compute MATLAB strides
            vector<size_t> strides_matlab(out_ndim);
            strides_matlab[0] = 1;
            for (mwSize i = 1; i < out_ndim; ++i) {
                strides_matlab[i] = strides_matlab[i-1] * out_dims[i-1];
            }
            
            // Copy with dimension reordering
            vector<size_t> indices(shape_ducc.size(), 0);
            size_t ndim = shape_ducc.size();
            
            for (size_t i = 0; i < nelem; ++i) {
                size_t idx_matlab = matlabLinearIndex(indices.data(), strides_matlab.data(), out_ndim);
                out_data[idx_matlab] = out_buffer[i];
                if (i < nelem - 1) {
                    incrementIndices(indices.data(), shape_ducc.data(), ndim);
                }
            }
            
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
            cfmav<float> in_view(in_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            vfmav<float> out_view(out_buffer.data(), shape_ducc, vector<ptrdiff_t>());
            
            // Compute normalization factor
            float fct = computeNormFactor<float>(inorm, shape_ducc, axes, type);
            bool ortho = (inorm == 1);
            
            // Perform DCT
            dct(in_view, out_view, axes, type, fct, ortho, nthreads);
            
            // Copy output from buffer to MATLAB (real data)
            float *out_data = static_cast<float *>(mxGetData(out_arr));
            mwSize out_ndim = shape_ducc.size();
            const mwSize *out_dims = mxGetDimensions(out_arr);
            
            // Pre-compute MATLAB strides
            vector<size_t> strides_matlab(out_ndim);
            strides_matlab[0] = 1;
            for (mwSize i = 1; i < out_ndim; ++i) {
                strides_matlab[i] = strides_matlab[i-1] * out_dims[i-1];
            }
            
            // Copy with dimension reordering
            vector<size_t> indices(shape_ducc.size(), 0);
            size_t ndim = shape_ducc.size();
            
            for (size_t i = 0; i < nelem; ++i) {
                size_t idx_matlab = matlabLinearIndex(indices.data(), strides_matlab.data(), out_ndim);
                out_data[idx_matlab] = out_buffer[i];
                if (i < nelem - 1) {
                    incrementIndices(indices.data(), shape_ducc.data(), ndim);
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:FFT:DCT:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}



