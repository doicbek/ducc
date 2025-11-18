/*
 * DUCC0 SHT Analysis 2D MEX Interface
 * 
 * MATLAB MEX gateway for spherical harmonic analysis (map2alm) for 2D grids
 * 
 * Usage:
 *   alm = ducc0_sht_analysis_2d_mex(map, lmax, spin, geometry, mmax, phi0, ringfactor, nthreads)
 * 
 * Inputs:
 *   map: Map data (real array, shape [nmaps, ntheta, nphi])
 *   lmax: Maximum multipole order l
 *   spin: Spin value (0, 1, or 2)
 *   geometry: Grid geometry string ('CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2')
 *   mmax: Maximum m order (optional, default: lmax)
 *   phi0: Phi offset in radians (optional, default: 0)
 *   ringfactor: Ring factors (optional, size [ntheta])
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   alm: Spherical harmonic coefficients (complex array, shape [ncomp, nalm])
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/sht/sht.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>
#include <string>
#include <array>

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

// Get number of map components based on spin
size_t get_nmaps(size_t spin)
{
    return (spin == 0) ? 1 : 2;
}

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
        if (nrhs < 4) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:InputError", 
                "At least 4 inputs required: map, lmax, spin, geometry");
        }
        
        const mxArray *map_arr = prhs[0];
        if (mxIsEmpty(map_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:InputError", 
                "map array cannot be empty");
        }
        
        // Parse required parameters
        size_t lmax = (size_t)mxGetScalar(prhs[1]);
        size_t spin = (size_t)mxGetScalar(prhs[2]);
        string geometry = getStringParam(prhs[3]);
        
        // Parse optional parameters
        size_t mmax = getOptionalParam<size_t>(nrhs > 4 ? prhs[4] : nullptr, lmax);
        double phi0 = getOptionalParam<double>(nrhs > 5 ? prhs[5] : nullptr, 0.0);
        const mxArray *ringfactor_arr = (nrhs > 6) ? prhs[6] : nullptr;
        size_t nthreads = getOptionalParam<size_t>(nrhs > 7 ? prhs[7] : nullptr, 0);
        
        // Get input dimensions
        mwSize ndim = mxGetNumberOfDimensions(map_arr);
        const mwSize *dims = mxGetDimensions(map_arr);
        
        if (ndim != 3) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:InputError", 
                "map must be 3D array [nmaps, ntheta, nphi]");
        }
        
        size_t nmaps = dims[0];
        size_t ntheta = dims[1];
        size_t nphi = dims[2];
        
        // Check number of map components
        size_t nmaps_expected = get_nmaps(spin);
        if (nmaps != nmaps_expected) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:InputError", 
                "map has wrong number of components");
        }
        
        // Determine ncomp (same as nmaps for analysis)
        size_t ncomp = nmaps;
        size_t nalm_expected = ((mmax+1)*(mmax+2))/2 + (mmax+1)*(lmax-mmax);
        
        // Build mstart array
        vector<size_t> mstart = build_mstart(lmax, mmax);
        
        // Get ringfactor if provided
        vector<double> ringfactor;
        if (ringfactor_arr != nullptr && !mxIsEmpty(ringfactor_arr)) {
            size_t nrf = mxGetNumberOfElements(ringfactor_arr);
            if (nrf != ntheta) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:InputError", 
                    "ringfactor must have size ntheta");
            }
            double *rf_data = mxGetPr(ringfactor_arr);
            ringfactor.assign(rf_data, rf_data + nrf);
        } else {
            ringfactor.assign(ntheta, 1.0);
        }
        
        // Determine data type
        mxClassID class_id = mxGetClassID(map_arr);
        bool is_complex = mxIsComplex(map_arr);
        
        if (is_complex) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:InputError", 
                "map must be real");
        }
        
        // Create output alm array (complex)
        mwSize alm_dims[2] = {ncomp, nalm_expected};
        mxArray *alm_arr = mxCreateNumericArray(2, alm_dims, class_id, mxCOMPLEX);
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Real double input -> complex double output
            vector<double> map_buffer;
            vector<complex<double>> alm_buffer;
            
            // Prepare map shape for DUCC (row-major): [nmaps, ntheta, nphi]
            size_t map_nelem = nmaps * ntheta * nphi;
            map_buffer.resize(map_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const double *map_data = mxGetPr(map_arr);
            for (size_t imap = 0; imap < nmaps; ++imap) {
                for (size_t itheta = 0; itheta < ntheta; ++itheta) {
                    for (size_t iphi = 0; iphi < nphi; ++iphi) {
                        size_t idx_matlab = imap + itheta * nmaps + iphi * nmaps * ntheta; // Column-major
                        size_t idx_ducc = imap * ntheta * nphi + itheta * nphi + iphi; // Row-major
                        map_buffer[idx_ducc] = map_data[idx_matlab];
                    }
                }
            }
            
            array<size_t,3> map_shape = {nmaps, ntheta, nphi};
            cmav<double,3> map_view(map_buffer.data(), map_shape);
            
            // Prepare alm buffer and view
            size_t alm_nelem = ncomp * nalm_expected;
            alm_buffer.resize(alm_nelem);
            array<size_t,2> alm_shape = {ncomp, nalm_expected};
            vmav<complex<double>,2> alm_view(alm_buffer.data(), alm_shape);
            
            // Create mstart view
            array<size_t,1> mstart_shape = {mmax+1};
            cmav<size_t,1> mstart_view(mstart.data(), mstart_shape);
            
            // Create ringfactor view
            array<size_t,1> ringfactor_shape = {ntheta};
            cmav<double,1> ringfactor_view(ringfactor.data(), ringfactor_shape);
            
            // Perform analysis
            analysis_2d(alm_view, map_view, spin, lmax, mstart_view, 1, 
                       geometry, phi0, ringfactor_view, nthreads);
            
            // Copy alm from buffer to MATLAB
            double *alm_real = mxGetPr(alm_arr);
            double *alm_imag = mxGetPi(alm_arr);
            for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                    size_t idx_ducc = icomp * nalm_expected + ialm; // Row-major
                    size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                    alm_real[idx_matlab] = alm_buffer[idx_ducc].real();
                    alm_imag[idx_matlab] = alm_buffer[idx_ducc].imag();
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Real single input -> complex single output
            vector<float> map_buffer;
            vector<complex<float>> alm_buffer;
            
            // Prepare map shape for DUCC (row-major): [nmaps, ntheta, nphi]
            size_t map_nelem = nmaps * ntheta * nphi;
            map_buffer.resize(map_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const float *map_data = (const float *)mxGetData(map_arr);
            for (size_t imap = 0; imap < nmaps; ++imap) {
                for (size_t itheta = 0; itheta < ntheta; ++itheta) {
                    for (size_t iphi = 0; iphi < nphi; ++iphi) {
                        size_t idx_matlab = imap + itheta * nmaps + iphi * nmaps * ntheta; // Column-major
                        size_t idx_ducc = imap * ntheta * nphi + itheta * nphi + iphi; // Row-major
                        map_buffer[idx_ducc] = map_data[idx_matlab];
                    }
                }
            }
            
            array<size_t,3> map_shape = {nmaps, ntheta, nphi};
            cmav<float,3> map_view(map_buffer.data(), map_shape);
            
            // Prepare alm buffer and view
            size_t alm_nelem = ncomp * nalm_expected;
            alm_buffer.resize(alm_nelem);
            array<size_t,2> alm_shape = {ncomp, nalm_expected};
            vmav<complex<float>,2> alm_view(alm_buffer.data(), alm_shape);
            
            // Create mstart view
            array<size_t,1> mstart_shape = {mmax+1};
            cmav<size_t,1> mstart_view(mstart.data(), mstart_shape);
            
            // Create ringfactor view
            array<size_t,1> ringfactor_shape = {ntheta};
            cmav<double,1> ringfactor_view(ringfactor.data(), ringfactor_shape);
            
            // Perform analysis
            analysis_2d(alm_view, map_view, spin, lmax, mstart_view, 1, 
                       geometry, phi0, ringfactor_view, nthreads);
            
            // Copy alm from buffer to MATLAB
            float *alm_real = (float *)mxGetData(alm_arr);
            float *alm_imag = (float *)mxGetImagData(alm_arr);
            for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                    size_t idx_ducc = icomp * nalm_expected + ialm; // Row-major
                    size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                    alm_real[idx_matlab] = alm_buffer[idx_ducc].real();
                    alm_imag[idx_matlab] = alm_buffer[idx_ducc].imag();
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:Analysis2D:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = alm_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

