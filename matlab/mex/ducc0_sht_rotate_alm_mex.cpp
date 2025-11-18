/*
 * DUCC0 SHT Rotate ALM MEX Interface
 * 
 * MATLAB MEX gateway for rotating spherical harmonic coefficients
 * 
 * Usage:
 *   alm_out = ducc0_sht_rotate_alm_mex(alm_in, lmax, psi, theta, phi, mmax_in, mmax_out, nthreads)
 * 
 * Inputs:
 *   alm_in: Spherical harmonic coefficients (complex array, shape [ncomp, nalm] or [nalm])
 *   lmax: Maximum multipole order l
 *   psi: First rotation angle about z-axis (radians)
 *   theta: Second rotation angle about y-axis (radians)
 *   phi: Third rotation angle about z-axis (radians)
 *   mmax_in: Maximum m order in input (optional, default: lmax)
 *   mmax_out: Maximum m order in output (optional, default: lmax)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   alm_out: Rotated spherical harmonic coefficients (complex array, same shape as input)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/sht/sht.h"
#include "ducc0/sht/alm.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>
#include <string>
#include <cmath>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

// Build mstart array for given lmax and mmax
vector<size_t> build_mstart(size_t lmax, size_t mmax)
{
    vector<size_t> mstart(mmax + 1);
    for (size_t m = 0, idx = 0; m <= mmax; ++m) {
        mstart[m] = idx;
        idx += lmax + 1 - m;
    }
    return mstart;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 5) {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:InputError", 
                "At least 5 inputs required: alm_in, lmax, psi, theta, phi");
        }
        
        const mxArray *alm_in_arr = prhs[0];
        if (mxIsEmpty(alm_in_arr)) {
            plhs[0] = mxDuplicateArray(alm_in_arr);
            return;
        }
        
        if (!mxIsComplex(alm_in_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:InputError", 
                "Input alm must be complex");
        }
        
        // Parse parameters
        size_t lmax = (size_t)mxGetScalar(prhs[1]);
        double psi = mxGetScalar(prhs[2]);
        double theta = mxGetScalar(prhs[3]);
        double phi = mxGetScalar(prhs[4]);
        size_t mmax_in = getOptionalParam<size_t>(nrhs > 5 ? prhs[5] : nullptr, lmax);
        size_t mmax_out = getOptionalParam<size_t>(nrhs > 6 ? prhs[6] : nullptr, lmax);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 7 ? prhs[7] : nullptr, 0);
        
        if (mmax_in > lmax) {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:InputError", 
                "mmax_in must be <= lmax");
        }
        if (mmax_out > lmax) {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:InputError", 
                "mmax_out must be <= lmax");
        }
        
        // Get input dimensions
        mwSize ndim = mxGetNumberOfDimensions(alm_in_arr);
        const mwSize *dims = mxGetDimensions(alm_in_arr);
        
        // Determine ncomp and nalm
        size_t ncomp = 1;
        size_t nalm_in = 0;
        if (ndim == 1) {
            nalm_in = dims[0];
        } else if (ndim == 2) {
            ncomp = dims[0];
            nalm_in = dims[1];
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:InputError", 
                "alm_in must be 1D or 2D array");
        }
        
        // Calculate expected nalm
        size_t nalm_in_expected = ((mmax_in+1)*(mmax_in+2))/2 + (mmax_in+1)*(lmax-mmax_in);
        size_t nalm_out_expected = ((mmax_out+1)*(mmax_out+2))/2 + (mmax_out+1)*(lmax-mmax_out);
        
        if (nalm_in != nalm_in_expected) {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:InputError", 
                "alm_in size does not match lmax and mmax_in");
        }
        
        // Determine data type
        mxClassID class_id = mxGetClassID(alm_in_arr);
        
        // Create output array (same shape as input, but may have different nalm)
        mwSize *out_dims = new mwSize[ndim];
        if (ndim == 1) {
            out_dims[0] = nalm_out_expected;
        } else {
            out_dims[0] = ncomp;
            out_dims[1] = nalm_out_expected;
        }
        mxArray *alm_out_arr = mxCreateNumericArray(ndim, out_dims, class_id, mxCOMPLEX);
        delete[] out_dims;
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Complex double input -> complex double output
            vector<complex<double>> alm_in_buffer;
            vector<complex<double>> alm_out_buffer;
            
            size_t alm_in_nelem = ncomp * nalm_in_expected;
            size_t alm_out_nelem = ncomp * nalm_out_expected;
            
            alm_in_buffer.resize(alm_in_nelem);
            alm_out_buffer.resize(alm_out_nelem);
            
            // Convert MATLAB alm_in to DUCC format
            const double *real_data = mxGetPr(alm_in_arr);
            const double *imag_data = mxGetPi(alm_in_arr);
            if (ndim == 1) {
                for (size_t ialm = 0; ialm < nalm_in_expected; ++ialm) {
                    alm_in_buffer[ialm] = complex<double>(real_data[ialm], imag_data[ialm]);
                }
            } else {
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_in_expected; ++ialm) {
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        size_t idx_ducc = icomp * nalm_in_expected + ialm; // Row-major
                        alm_in_buffer[idx_ducc] = complex<double>(real_data[idx_matlab], imag_data[idx_matlab]);
                    }
                }
            }
            
            // Create Alm_Base objects
            Alm_Base base_in(lmax, mmax_in);
            Alm_Base base_out(lmax, mmax_out);
            
            // Create DUCC array views
            array<size_t,2> alm_in_shape = {ncomp, nalm_in_expected};
            array<size_t,2> alm_out_shape = {ncomp, nalm_out_expected};
            cmav<complex<double>,2> alm_in_view(alm_in_buffer.data(), alm_in_shape);
            vmav<complex<double>,2> alm_out_view(alm_out_buffer.data(), alm_out_shape);
            
            // Perform rotation
            rotate_alm(base_in, alm_in_view, base_out, alm_out_view, psi, theta, phi, nthreads);
            
            // Convert DUCC alm_out to MATLAB format
            double *alm_out_real = mxGetPr(alm_out_arr);
            double *alm_out_imag = mxGetPi(alm_out_arr);
            if (ndim == 1) {
                for (size_t ialm = 0; ialm < nalm_out_expected; ++ialm) {
                    alm_out_real[ialm] = alm_out_buffer[ialm].real();
                    alm_out_imag[ialm] = alm_out_buffer[ialm].imag();
                }
            } else {
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_out_expected; ++ialm) {
                        size_t idx_ducc = icomp * nalm_out_expected + ialm; // Row-major
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        alm_out_real[idx_matlab] = alm_out_buffer[idx_ducc].real();
                        alm_out_imag[idx_matlab] = alm_out_buffer[idx_ducc].imag();
                    }
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Complex single input -> complex single output
            vector<complex<float>> alm_in_buffer;
            vector<complex<float>> alm_out_buffer;
            
            size_t alm_in_nelem = ncomp * nalm_in_expected;
            size_t alm_out_nelem = ncomp * nalm_out_expected;
            
            alm_in_buffer.resize(alm_in_nelem);
            alm_out_buffer.resize(alm_out_nelem);
            
            // Convert MATLAB alm_in to DUCC format
            const float *real_data = (const float *)mxGetData(alm_in_arr);
            const float *imag_data = (const float *)mxGetImagData(alm_in_arr);
            if (ndim == 1) {
                for (size_t ialm = 0; ialm < nalm_in_expected; ++ialm) {
                    alm_in_buffer[ialm] = complex<float>(real_data[ialm], imag_data[ialm]);
                }
            } else {
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_in_expected; ++ialm) {
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        size_t idx_ducc = icomp * nalm_in_expected + ialm; // Row-major
                        alm_in_buffer[idx_ducc] = complex<float>(real_data[idx_matlab], imag_data[idx_matlab]);
                    }
                }
            }
            
            // Create Alm_Base objects
            Alm_Base base_in(lmax, mmax_in);
            Alm_Base base_out(lmax, mmax_out);
            
            // Create DUCC array views
            array<size_t,2> alm_in_shape = {ncomp, nalm_in_expected};
            array<size_t,2> alm_out_shape = {ncomp, nalm_out_expected};
            cmav<complex<float>,2> alm_in_view(alm_in_buffer.data(), alm_in_shape);
            vmav<complex<float>,2> alm_out_view(alm_out_buffer.data(), alm_out_shape);
            
            // Perform rotation
            rotate_alm(base_in, alm_in_view, base_out, alm_out_view, psi, theta, phi, nthreads);
            
            // Convert DUCC alm_out to MATLAB format
            float *alm_out_real = (float *)mxGetData(alm_out_arr);
            float *alm_out_imag = (float *)mxGetImagData(alm_out_arr);
            if (ndim == 1) {
                for (size_t ialm = 0; ialm < nalm_out_expected; ++ialm) {
                    alm_out_real[ialm] = alm_out_buffer[ialm].real();
                    alm_out_imag[ialm] = alm_out_buffer[ialm].imag();
                }
            } else {
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_out_expected; ++ialm) {
                        size_t idx_ducc = icomp * nalm_out_expected + ialm; // Row-major
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        alm_out_real[idx_matlab] = alm_out_buffer[idx_ducc].real();
                        alm_out_imag[idx_matlab] = alm_out_buffer[idx_ducc].imag();
                    }
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:RotateALM:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = alm_out_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

