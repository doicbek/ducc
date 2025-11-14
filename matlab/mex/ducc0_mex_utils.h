/*
 * DUCC0 MATLAB MEX Utilities
 * 
 * This file provides utilities for converting between MATLAB mxArray
 * and DUCC's multi-dimensional array views.
 * 
 * Key features:
 * - Handles MATLAB's column-major storage vs DUCC's row-major expectations
 * - Converts MATLAB's separate real/imaginary arrays to interleaved complex
 * - Supports automatic dimension reordering
 * - Provides error handling for MATLAB
 */

#ifndef DUCC0_MEX_UTILS_H
#define DUCC0_MEX_UTILS_H

#include "mex.h"
#include "ducc0/infra/mav.h"
#include "ducc0/infra/error_handling.h"
#include "ducc0/bindings/typecode.h"
#include <vector>
#include <complex>
#include <memory>
#include <algorithm>
#include <type_traits>

namespace ducc0_mex {

using namespace ducc0;
using namespace std;

// Error handler that throws MATLAB error
inline void handleDuccError(const exception &e)
{
    mexErrMsgIdAndTxt("DUCC0:MEXError", "%s", e.what());
}

// Convert MATLAB dimensions (column-major) to vector (row-major for DUCC)
inline vector<size_t> matlabDimsToVector(const mwSize *dims, mwSize ndim)
{
    vector<size_t> result(ndim);
    // MATLAB stores dimensions in column-major order [rows, cols, ...]
    // DUCC expects row-major order, so we reverse
    for (mwSize i = 0; i < ndim; ++i) {
        result[ndim - 1 - i] = dims[i];
    }
    return result;
}

// Convert vector dimensions (row-major) to MATLAB dimensions (column-major)
inline void vectorDimsToMatlab(const vector<size_t> &dims, mwSize *out_dims)
{
    mwSize ndim = dims.size();
    // Reverse to convert from row-major (DUCC) to column-major (MATLAB)
    for (size_t i = 0; i < dims.size(); ++i) {
        out_dims[dims.size() - 1 - i] = dims[i];
    }
}

// Convert axes from MATLAB 1-based column-major to DUCC 0-based row-major
inline vector<size_t> convertAxes(const mxArray *axes_arr, size_t ndim)
{
    vector<size_t> axes;
    
    if (axes_arr == nullptr || mxIsEmpty(axes_arr)) {
        // Default: all axes in row-major order
        for (size_t i = 0; i < ndim; ++i) {
            axes.push_back(i);
        }
    } else {
        size_t naxes = mxGetNumberOfElements(axes_arr);
        double *axes_data = mxGetPr(axes_arr);
        
        for (size_t i = 0; i < naxes; ++i) {
            // MATLAB uses 1-based indexing, column-major
            size_t ax_matlab = (size_t)axes_data[i] - 1;
            // Convert to 0-based row-major
            size_t ax_ducc = ndim - 1 - ax_matlab;
            if (ax_ducc >= ndim) {
                MR_fail("Axis index out of range");
            }
            axes.push_back(ax_ducc);
        }
    }
    
    return axes;
}

// Helper to compute MATLAB linear index from row-major indices
inline size_t matlabLinearIndex(const vector<size_t> &indices_rowmajor,
                                 const mwSize *dims_matlab, mwSize ndim)
{
    size_t idx = 0;
    size_t stride = 1;
    // MATLAB uses column-major, so reverse the indices
    for (mwSize i = 0; i < ndim; ++i) {
        idx += indices_rowmajor[ndim - 1 - i] * stride;
        stride *= dims_matlab[i];
    }
    return idx;
}

// Helper to increment row-major indices (rightmost dimension first)
inline void incrementIndices(vector<size_t> &indices, const vector<size_t> &shape)
{
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i]++;
        if (indices[i] < shape[i]) {
            return;  // Still valid, done
        }
        indices[i] = 0;  // Overflow, carry to next dimension
    }
    // All indices overflowed - this shouldn't happen if called correctly
}

// Copy data from MATLAB array to temporary buffer with proper layout
// This handles column-major to row-major conversion and complex interleaving
template<typename T>
void copyMatlabToBuffer(const mxArray *arr, T *buffer, const vector<size_t> &shape_ducc)
{
    const void *real_data = mxGetData(arr);
    const void *imag_data = mxIsComplex(arr) ? mxGetImagData(arr) : nullptr;
    
    mwSize ndim_matlab = mxGetNumberOfDimensions(arr);
    const mwSize *dims_matlab = mxGetDimensions(arr);
    
    // Calculate total elements
    size_t nelem = 1;
    for (size_t s : shape_ducc) {
        nelem *= s;
    }
    
    if (imag_data == nullptr) {
        // Real data - copy with dimension reordering
        const T *real = static_cast<const T *>(real_data);
        
        if (ndim_matlab == 1) {
            // 1D - just copy (no reordering needed)
            memcpy(buffer, real, nelem * sizeof(T));
        } else {
            // Multi-dimensional - need to reorder
            vector<size_t> indices(shape_ducc.size(), 0);
            for (size_t i = 0; i < nelem; ++i) {
                // Calculate MATLAB linear index from row-major indices
                size_t idx_matlab = matlabLinearIndex(indices, dims_matlab, ndim_matlab);
                buffer[i] = real[idx_matlab];
                
                // Increment indices for next iteration (except for last element)
                if (i < nelem - 1) {
                    incrementIndices(indices, shape_ducc);
                }
            }
        }
    } else {
        // Complex data - interleave real and imaginary parts
        using real_t = typename T::value_type;
        const real_t *real = static_cast<const real_t *>(real_data);
        const real_t *imag = static_cast<const real_t *>(imag_data);
        
        if (ndim_matlab == 1) {
            // 1D - just interleave (no reordering needed)
            for (size_t i = 0; i < nelem; ++i) {
                buffer[i] = T(real[i], imag[i]);
            }
        } else {
            // Multi-dimensional - need to reorder and interleave
            vector<size_t> indices(shape_ducc.size(), 0);
            for (size_t i = 0; i < nelem; ++i) {
                // Calculate MATLAB linear index from row-major indices
                size_t idx_matlab = matlabLinearIndex(indices, dims_matlab, ndim_matlab);
                buffer[i] = T(real[idx_matlab], imag[idx_matlab]);
                
                // Increment indices for next iteration (except for last element)
                if (i < nelem - 1) {
                    incrementIndices(indices, shape_ducc);
                }
            }
        }
    }
}

// Copy data from buffer to MATLAB array with proper layout
template<typename T>
void copyBufferToMatlab(const T *buffer, mxArray *arr, const vector<size_t> &shape_ducc)
{
    void *real_data = mxGetData(arr);
    void *imag_data = mxIsComplex(arr) ? mxGetImagData(arr) : nullptr;
    
    mwSize ndim_matlab = mxGetNumberOfDimensions(arr);
    const mwSize *dims_matlab = mxGetDimensions(arr);
    
    // Calculate total elements
    size_t nelem = 1;
    for (size_t s : shape_ducc) {
        nelem *= s;
    }
    
    if (imag_data == nullptr) {
        // Real data - copy with dimension reordering
        T *real = static_cast<T *>(real_data);
        
        if (ndim_matlab == 1) {
            // 1D - just copy (no reordering needed)
            memcpy(real, buffer, nelem * sizeof(T));
        } else {
            // Multi-dimensional - need to reorder
            vector<size_t> indices(shape_ducc.size(), 0);
            for (size_t i = 0; i < nelem; ++i) {
                // Calculate MATLAB linear index from row-major indices
                size_t idx_matlab = matlabLinearIndex(indices, dims_matlab, ndim_matlab);
                real[idx_matlab] = buffer[i];
                
                // Increment indices for next iteration (except for last element)
                if (i < nelem - 1) {
                    incrementIndices(indices, shape_ducc);
                }
            }
        }
    } else {
        // Complex data - deinterleave real and imaginary parts
        using real_t = typename T::value_type;
        real_t *real = static_cast<real_t *>(real_data);
        real_t *imag = static_cast<real_t *>(imag_data);
        
        if (ndim_matlab == 1) {
            // 1D - just deinterleave (no reordering needed)
            for (size_t i = 0; i < nelem; ++i) {
                real[i] = buffer[i].real();
                imag[i] = buffer[i].imag();
            }
        } else {
            // Multi-dimensional - need to reorder and deinterleave
            vector<size_t> indices(shape_ducc.size(), 0);
            for (size_t i = 0; i < nelem; ++i) {
                // Calculate MATLAB linear index from row-major indices
                size_t idx_matlab = matlabLinearIndex(indices, dims_matlab, ndim_matlab);
                real[idx_matlab] = buffer[i].real();
                imag[idx_matlab] = buffer[i].imag();
                
                // Increment indices for next iteration (except for last element)
                if (i < nelem - 1) {
                    incrementIndices(indices, shape_ducc);
                }
            }
        }
    }
}

// Get optional parameter with default value
template<typename T>
T getOptionalParam(const mxArray *param, T default_value)
{
    if (param == nullptr || mxIsEmpty(param)) {
        return default_value;
    }
    return static_cast<T>(mxGetScalar(param));
}

// Get string parameter
inline string getStringParam(const mxArray *param, const string &default_value = "")
{
    if (param == nullptr || mxIsEmpty(param)) {
        return default_value;
    }
    
    if (!mxIsChar(param)) {
        MR_fail("Parameter must be a string");
    }
    
    char *str = mxArrayToString(param);
    if (str == nullptr) {
        MR_fail("Failed to convert parameter to string");
    }
    
    string result(str);
    mxFree(str);
    return result;
}

} // namespace ducc0_mex

#endif // DUCC0_MEX_UTILS_H
