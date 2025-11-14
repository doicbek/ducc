/*
 * DUCC0 Misc VDOT MEX Interface
 * 
 * MATLAB MEX gateway for computing scalar product with long double accumulation
 * 
 * Usage:
 *   result = ducc0_misc_vdot_mex(a, b)
 * 
 * Inputs:
 *   a: First array (numeric, any shape)
 *   b: Second array (numeric, same shape as a)
 * 
 * Output:
 *   result: Scalar product (double or complex)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/infra/mav.h"
#include "ducc0/infra/misc_utils.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>
#include <cmath>

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 2) {
            mexErrMsgIdAndTxt("DUCC0:Misc:VDOT:InputError", 
                "At least 2 inputs required: a, b");
        }
        
        const mxArray *a_arr = prhs[0];
        const mxArray *b_arr = prhs[1];
        
        if (mxIsEmpty(a_arr) || mxIsEmpty(b_arr)) {
            mexErrMsgIdAndTxt("DUCC0:Misc:VDOT:InputError", 
                "a and b cannot be empty");
        }
        
        // Check dimensions match
        mwSize a_ndim = mxGetNumberOfDimensions(a_arr);
        mwSize b_ndim = mxGetNumberOfDimensions(b_arr);
        const mwSize *a_dims = mxGetDimensions(a_arr);
        const mwSize *b_dims = mxGetDimensions(b_arr);
        
        if (a_ndim != b_ndim) {
            mexErrMsgIdAndTxt("DUCC0:Misc:VDOT:InputError", 
                "a and b must have the same number of dimensions");
        }
        
        for (mwSize i = 0; i < a_ndim; ++i) {
            if (a_dims[i] != b_dims[i]) {
                mexErrMsgIdAndTxt("DUCC0:Misc:VDOT:InputError", 
                    "a and b must have the same shape");
            }
        }
        
        size_t nelem = mxGetNumberOfElements(a_arr);
        
        // Determine data type
        mxClassID a_class = mxGetClassID(a_arr);
        mxClassID b_class = mxGetClassID(b_arr);
        bool a_complex = mxIsComplex(a_arr);
        bool b_complex = mxIsComplex(b_arr);
        
        // Compute scalar product with long double accumulation
        long double acc_real = 0.0;
        long double acc_imag = 0.0;
        
        if (a_class == mxDOUBLE_CLASS && b_class == mxDOUBLE_CLASS) {
            if (a_complex && b_complex) {
                const double *a_real = mxGetPr(a_arr);
                const double *a_imag = mxGetPi(a_arr);
                const double *b_real = mxGetPr(b_arr);
                const double *b_imag = mxGetPi(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    complex<long double> a_val(a_real[i], a_imag[i]);
                    complex<long double> b_val(b_real[i], b_imag[i]);
                    complex<long double> prod = conj(a_val) * b_val;
                    acc_real += prod.real();
                    acc_imag += prod.imag();
                }
            } else if (a_complex) {
                const double *a_real = mxGetPr(a_arr);
                const double *a_imag = mxGetPi(a_arr);
                const double *b_real = mxGetPr(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    complex<long double> a_val(a_real[i], a_imag[i]);
                    complex<long double> prod = conj(a_val) * b_real[i];
                    acc_real += prod.real();
                    acc_imag += prod.imag();
                }
            } else if (b_complex) {
                const double *a_real = mxGetPr(a_arr);
                const double *b_real = mxGetPr(b_arr);
                const double *b_imag = mxGetPi(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    complex<long double> b_val(b_real[i], b_imag[i]);
                    complex<long double> prod = a_real[i] * b_val;
                    acc_real += prod.real();
                    acc_imag += prod.imag();
                }
            } else {
                const double *a_real = mxGetPr(a_arr);
                const double *b_real = mxGetPr(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    acc_real += a_real[i] * b_real[i];
                }
            }
        } else if (a_class == mxSINGLE_CLASS && b_class == mxSINGLE_CLASS) {
            if (a_complex && b_complex) {
                const float *a_real = (const float *)mxGetData(a_arr);
                const float *a_imag = (const float *)mxGetImagData(a_arr);
                const float *b_real = (const float *)mxGetData(b_arr);
                const float *b_imag = (const float *)mxGetImagData(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    complex<long double> a_val(a_real[i], a_imag[i]);
                    complex<long double> b_val(b_real[i], b_imag[i]);
                    complex<long double> prod = conj(a_val) * b_val;
                    acc_real += prod.real();
                    acc_imag += prod.imag();
                }
            } else if (a_complex) {
                const float *a_real = (const float *)mxGetData(a_arr);
                const float *a_imag = (const float *)mxGetImagData(a_arr);
                const float *b_real = (const float *)mxGetData(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    complex<long double> a_val(a_real[i], a_imag[i]);
                    complex<long double> prod = conj(a_val) * b_real[i];
                    acc_real += prod.real();
                    acc_imag += prod.imag();
                }
            } else if (b_complex) {
                const float *a_real = (const float *)mxGetData(a_arr);
                const float *b_real = (const float *)mxGetData(b_arr);
                const float *b_imag = (const float *)mxGetImagData(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    complex<long double> b_val(b_real[i], b_imag[i]);
                    complex<long double> prod = a_real[i] * b_val;
                    acc_real += prod.real();
                    acc_imag += prod.imag();
                }
            } else {
                const float *a_real = (const float *)mxGetData(a_arr);
                const float *b_real = (const float *)mxGetData(b_arr);
                
                for (size_t i = 0; i < nelem; ++i) {
                    acc_real += (long double)a_real[i] * (long double)b_real[i];
                }
            }
        } else {
            mexErrMsgIdAndTxt("DUCC0:Misc:VDOT:TypeError", 
                "a and b must have compatible types (double or single)");
        }
        
        // Create output
        if (acc_imag == 0.0) {
            // Real result
            plhs[0] = mxCreateDoubleScalar((double)acc_real);
        } else {
            // Complex result
            mxArray *result_arr = mxCreateDoubleMatrix(1, 1, mxCOMPLEX);
            double *result_real = mxGetPr(result_arr);
            double *result_imag = mxGetPi(result_arr);
            result_real[0] = (double)acc_real;
            result_imag[0] = (double)acc_imag;
            plhs[0] = result_arr;
        }
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

