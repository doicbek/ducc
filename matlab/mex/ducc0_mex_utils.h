/*
 * DUCC0 MATLAB MEX Utilities
 * 
 * This file provides utilities for converting between MATLAB mxArray
 * and DUCC's ArrayDescriptor format.
 */

#ifndef DUCC0_MEX_UTILS_H
#define DUCC0_MEX_UTILS_H

#include "mex.h"
#include "ducc0/bindings/array_descriptor.h"
#include "ducc0/bindings/typecode.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>

namespace ducc0_mex {

using namespace ducc0;
using namespace std;

// Convert MATLAB mxArray to DUCC ArrayDescriptor
inline ArrayDescriptor mxArrayToDescriptor(const mxArray *arr)
{
    if (arr == nullptr)
        MR_fail("Null mxArray pointer");
    
    ArrayDescriptor desc;
    desc.ndim = mxGetNumberOfDimensions(arr);
    MR_assert(desc.ndim <= ArrayDescriptor::maxdim, "Array has too many dimensions");
    
    const mwSize *dims = mxGetDimensions(arr);
    void *data = mxGetData(arr);
    
    // Determine typecode from MATLAB class
    mxClassID class_id = mxGetClassID(arr);
    bool is_complex = mxIsComplex(arr);
    
    switch (class_id) {
        case mxDOUBLE_CLASS:
            desc.typecode = is_complex ? Typecode<complex<double>>::value : Typecode<double>::value;
            break;
        case mxSINGLE_CLASS:
            desc.typecode = is_complex ? Typecode<complex<float>>::value : Typecode<float>::value;
            break;
        case mxINT32_CLASS:
            desc.typecode = Typecode<int32_t>::value;
            break;
        case mxINT64_CLASS:
            desc.typecode = Typecode<int64_t>::value;
            break;
        default:
            MR_fail("Unsupported MATLAB data type");
    }
    
    // MATLAB uses column-major, DUCC uses row-major for ArrayDescriptor
    // We need to reverse dimensions
    for (size_t i = 0; i < desc.ndim; ++i) {
        desc.shape[i] = dims[desc.ndim - 1 - i];
    }
    
    // Calculate strides (column-major for MATLAB)
    ptrdiff_t stride = 1;
    for (int i = desc.ndim - 1; i >= 0; --i) {
        desc.stride[i] = stride;
        stride *= desc.shape[i];
    }
    
    // MATLAB stores complex arrays with separate real/imag pointers
    // We'll handle this in the calling code by creating interleaved storage
    // For now, just store the real part pointer - complex conversion happens elsewhere
    desc.data = data;
    
    return desc;
}

// Create MATLAB mxArray from DUCC ArrayDescriptor
inline mxArray* descriptorToMxArray(const ArrayDescriptor &desc)
{
    mxClassID class_id;
    bool is_complex = false;
    
    // Determine MATLAB class from typecode
    if (isTypecode<complex<double>>(desc.typecode)) {
        class_id = mxDOUBLE_CLASS;
        is_complex = true;
    } else if (isTypecode<complex<float>>(desc.typecode)) {
        class_id = mxSINGLE_CLASS;
        is_complex = true;
    } else if (isTypecode<double>(desc.typecode)) {
        class_id = mxDOUBLE_CLASS;
    } else if (isTypecode<float>(desc.typecode)) {
        class_id = mxSINGLE_CLASS;
    } else if (isTypecode<int32_t>(desc.typecode)) {
        class_id = mxINT32_CLASS;
    } else if (isTypecode<int64_t>(desc.typecode)) {
        class_id = mxINT64_CLASS;
    } else {
        MR_fail("Unsupported typecode for MATLAB conversion");
    }
    
    // Convert shape from row-major (DUCC) to column-major (MATLAB)
    mwSize *dims = new mwSize[desc.ndim];
    for (size_t i = 0; i < desc.ndim; ++i) {
        dims[i] = desc.shape[desc.ndim - 1 - i];
    }
    
    mxArray *arr = mxCreateNumericArray(desc.ndim, dims, class_id, 
                                        is_complex ? mxCOMPLEX : mxREAL);
    
    // Copy data
    size_t element_size = typeSize(desc.typecode);
    size_t total_elements = 1;
    for (size_t i = 0; i < desc.ndim; ++i) {
        total_elements *= desc.shape[i];
    }
    
    void *matlab_data = mxGetData(arr);
    memcpy(matlab_data, desc.data, total_elements * element_size);
    
    if (is_complex) {
        void *imag_data = mxGetImagData(arr);
        // For complex, we need to handle interleaved vs separate storage
        // This is a simplified version - may need adjustment
        memcpy(imag_data, (char*)desc.data + total_elements * element_size / 2, 
               total_elements * element_size / 2);
    }
    
    delete[] dims;
    return arr;
}

// Helper to get axes vector from MATLAB input
inline vector<size_t> getAxesFromInput(const mxArray *axes_arr, size_t ndim)
{
    vector<size_t> axes;
    
    if (axes_arr == nullptr || mxIsEmpty(axes_arr)) {
        // Default: all axes
        for (size_t i = 0; i < ndim; ++i) {
            axes.push_back(i);
        }
    } else {
        size_t naxes = mxGetNumberOfElements(axes_arr);
        double *axes_data = mxGetPr(axes_arr);
        
        for (size_t i = 0; i < naxes; ++i) {
            size_t ax = (size_t)axes_data[i];
            // Convert from 1-based (MATLAB) to 0-based, and reverse for row-major
            ax = ndim - 1 - (ax - 1);
            MR_assert(ax < ndim, "Axis index out of range");
            axes.push_back(ax);
        }
    }
    
    return axes;
}

// Error handler that throws MATLAB error
inline void handleDuccError(const exception &e)
{
    mexErrMsgIdAndTxt("DUCC0:MEXError", "%s", e.what());
}

} // namespace ducc0_mex

#endif // DUCC0_MEX_UTILS_H

