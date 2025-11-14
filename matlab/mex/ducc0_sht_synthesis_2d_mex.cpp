/*
 * DUCC0 SHT Synthesis 2D MEX Interface
 * 
 * MATLAB MEX gateway for spherical harmonic synthesis (alm2map) for 2D grids
 * 
 * Usage:
 *   map = ducc0_sht_synthesis_2d_mex(alm, lmax, spin, geometry, ntheta, nphi, mmax, phi0, ringfactor, mode, nthreads)
 * 
 * Inputs:
 *   alm: Spherical harmonic coefficients (complex array, shape [ncomp, nalm] or [nalm])
 *   lmax: Maximum multipole order l
 *   spin: Spin value (0, 1, or 2)
 *   geometry: Grid geometry string ('CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2')
 *   ntheta: Number of theta rings (optional)
 *   nphi: Number of phi pixels per ring (optional)
 *   mmax: Maximum m order (optional, default: lmax)
 *   phi0: Phi offset in radians (optional, default: 0)
 *   ringfactor: Ring factors (optional, size [ntheta])
 *   mode: Transform mode ('STANDARD', 'GRAD_ONLY', 'DERIV1') (optional, default: 'STANDARD')
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   map: Synthesized map (real array, shape [nmaps, ntheta, nphi])
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/sht/sht.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>
#include <string>

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

// Convert SHT mode string to enum
SHT_mode get_sht_mode(const string &mode_str)
{
    if (mode_str == "STANDARD") return STANDARD;
    if (mode_str == "GRAD_ONLY") return GRAD_ONLY;
    if (mode_str == "DERIV1") return DERIV1;
    MR_fail("Unknown SHT mode: " + mode_str);
    return STANDARD;
}

// Get number of map components based on spin and mode
size_t get_nmaps(size_t spin, SHT_mode mode)
{
    return (spin == 0) ? 1 : 2;
}

// Get number of alm components based on spin and mode
size_t get_nalm(size_t spin, SHT_mode mode)
{
    if (spin == 0) return 1;
    return (mode == STANDARD) ? 2 : 1;
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

// Determine default ntheta based on geometry and lmax
size_t get_default_ntheta(const string &geometry, size_t lmax)
{
    if (geometry == "CC") return lmax + 2;
    if (geometry == "DH") return 2 * lmax + 2;
    return lmax + 1;
}

// Determine default nphi based on mmax
size_t get_default_nphi(size_t mmax)
{
    return 2 * mmax + 2;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 4) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                "At least 4 inputs required: alm, lmax, spin, geometry");
        }
        
        const mxArray *alm_arr = prhs[0];
        if (mxIsEmpty(alm_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                "alm array cannot be empty");
        }
        
        // Parse required parameters
        size_t lmax = (size_t)mxGetScalar(prhs[1]);
        size_t spin = (size_t)mxGetScalar(prhs[2]);
        string geometry = getStringParam(prhs[3]);
        
        // Parse optional parameters
        size_t ntheta = getOptionalParam<size_t>(nrhs > 4 ? prhs[4] : nullptr, 0);
        size_t nphi = getOptionalParam<size_t>(nrhs > 5 ? prhs[5] : nullptr, 0);
        size_t mmax = getOptionalParam<size_t>(nrhs > 6 ? prhs[6] : nullptr, lmax);
        double phi0 = getOptionalParam<double>(nrhs > 7 ? prhs[7] : nullptr, 0.0);
        const mxArray *ringfactor_arr = (nrhs > 8) ? prhs[8] : nullptr;
        string mode_str = getStringParam(nrhs > 9 ? prhs[9] : nullptr, "STANDARD");
        size_t nthreads = getOptionalParam<size_t>(nrhs > 10 ? prhs[10] : nullptr, 0);
        
        // Convert mode string to enum
        SHT_mode mode = get_sht_mode(mode_str);
        
        // Get input dimensions
        mwSize ndim = mxGetNumberOfDimensions(alm_arr);
        const mwSize *dims = mxGetDimensions(alm_arr);
        
        if (ndim < 1 || ndim > 2) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                "alm must be 1D or 2D array");
        }
        
        // Determine ncomp and nalm
        size_t ncomp_expected = get_nalm(spin, mode);
        size_t nalm_expected = ((mmax+1)*(mmax+2))/2 + (mmax+1)*(lmax-mmax);
        
        size_t ncomp_in, nalm_in;
        if (ndim == 1) {
            // 1D alm - treat as single component
            ncomp_in = 1;
            nalm_in = dims[0];
            if (ncomp_expected != 1) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                    "1D alm array requires spin=0");
            }
            if (nalm_in != nalm_expected) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                    "alm size does not match expected size for given lmax and mmax");
            }
        } else {
            // 2D alm - [ncomp, nalm]
            ncomp_in = dims[0];
            nalm_in = dims[1];
            if (ncomp_in != ncomp_expected) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                    "alm has wrong number of components");
            }
            if (nalm_in != nalm_expected) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                    "alm has wrong size for given lmax and mmax");
            }
        }
        
        size_t ncomp = ncomp_in;
        
        // Determine default ntheta and nphi if not provided
        if (ntheta == 0) {
            ntheta = get_default_ntheta(geometry, lmax);
        }
        if (nphi == 0) {
            nphi = get_default_nphi(mmax);
        }
        
        // Get number of map components
        size_t nmaps = get_nmaps(spin, mode);
        
        // Build mstart array
        vector<size_t> mstart = build_mstart(lmax, mmax);
        
        // Get ringfactor if provided
        vector<double> ringfactor;
        if (ringfactor_arr != nullptr && !mxIsEmpty(ringfactor_arr)) {
            size_t nrf = mxGetNumberOfElements(ringfactor_arr);
            if (nrf != ntheta) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                    "ringfactor must have size ntheta");
            }
            double *rf_data = mxGetPr(ringfactor_arr);
            ringfactor.assign(rf_data, rf_data + nrf);
        } else {
            ringfactor.assign(ntheta, 1.0);
        }
        
        // Determine data type
        mxClassID class_id = mxGetClassID(alm_arr);
        bool is_complex = mxIsComplex(alm_arr);
        
        if (!is_complex) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:InputError", 
                "alm must be complex");
        }
        
        // Create output map array (real, not complex)
        mwSize map_dims[3] = {nmaps, ntheta, nphi};
        mxArray *map_arr = mxCreateNumericArray(3, map_dims, class_id, mxREAL);
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Complex double input -> real double output
            vector<double> map_buffer;
            
            // Prepare alm shape for DUCC (row-major): [ncomp, nalm]
            size_t alm_nelem = ncomp * nalm_expected;
            vector<complex<double>> alm_reshaped(alm_nelem);
            // Reorder from MATLAB column-major to DUCC row-major
            const double *real_data = mxGetPr(alm_arr);
            const double *imag_data = mxGetPi(alm_arr);
            if (ndim == 1) {
                // 1D alm - just copy
                for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                    alm_reshaped[ialm] = complex<double>(real_data[ialm], imag_data[ialm]);
                }
            } else {
                // 2D alm - reorder
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        size_t idx_ducc = icomp * nalm_expected + ialm; // Row-major
                        alm_reshaped[idx_ducc] = complex<double>(real_data[idx_matlab], imag_data[idx_matlab]);
                    }
                }
            }
            
            cmav<complex<double>,2> alm_view(alm_reshaped.data(), alm_shape, vector<ptrdiff_t>());
            
            // Prepare map buffer and view - DUCC expects [nmaps, ntheta, nphi] in row-major
            size_t map_nelem = nmaps * ntheta * nphi;
            map_buffer.resize(map_nelem);
            vector<size_t> map_shape = {nmaps, ntheta, nphi};
            vmav<double,3> map_view(map_buffer.data(), map_shape, vector<ptrdiff_t>());
            
            // Create mstart view
            cmav<size_t,1> mstart_view(mstart.data(), {mmax+1}, vector<ptrdiff_t>());
            
            // Create ringfactor view
            cmav<double,1> ringfactor_view(ringfactor.data(), {ntheta}, vector<ptrdiff_t>());
            
            // Perform synthesis
            synthesis_2d(alm_view, map_view, spin, lmax, mstart_view, 1, 
                        geometry, phi0, ringfactor_view, nthreads, mode);
            
            // Copy map from buffer to MATLAB
            // DUCC outputs [nmaps, ntheta, nphi] in row-major
            // MATLAB expects [nmaps, ntheta, nphi] in column-major
            double *map_out = mxGetPr(map_arr);
            for (size_t imap = 0; imap < nmaps; ++imap) {
                for (size_t itheta = 0; itheta < ntheta; ++itheta) {
                    for (size_t iphi = 0; iphi < nphi; ++iphi) {
                        size_t idx_ducc = imap * ntheta * nphi + itheta * nphi + iphi; // Row-major
                        size_t idx_matlab = imap + itheta * nmaps + iphi * nmaps * ntheta; // Column-major
                        map_out[idx_matlab] = map_buffer[idx_ducc];
                    }
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Complex single input -> real single output
            vector<complex<float>> alm_buffer;
            vector<float> map_buffer;
            
            // Prepare alm shape for DUCC (row-major): [ncomp, nalm]
            vector<size_t> alm_shape = {ncomp, nalm_expected};
            size_t alm_nelem = ncomp * nalm_expected;
            alm_buffer.resize(alm_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const float *real_data = (const float *)mxGetData(alm_arr);
            const float *imag_data = (const float *)mxGetImagData(alm_arr);
            if (ndim == 1) {
                // 1D alm - just copy
                for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                    alm_buffer[ialm] = complex<float>(real_data[ialm], imag_data[ialm]);
                }
            } else {
                // 2D alm - reorder
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        size_t idx_ducc = icomp * nalm_expected + ialm; // Row-major
                        alm_buffer[idx_ducc] = complex<float>(real_data[idx_matlab], imag_data[idx_matlab]);
                    }
                }
            }
            
            cmav<complex<float>,2> alm_view(alm_buffer.data(), alm_shape, vector<ptrdiff_t>());
            
            // Prepare map buffer and view
            size_t map_nelem = nmaps * ntheta * nphi;
            map_buffer.resize(map_nelem);
            vector<size_t> map_shape = {nmaps, ntheta, nphi};
            vmav<float,3> map_view(map_buffer.data(), map_shape, vector<ptrdiff_t>());
            
            // Create mstart view
            cmav<size_t,1> mstart_view(mstart.data(), {mmax+1}, vector<ptrdiff_t>());
            
            // Create ringfactor view
            cmav<double,1> ringfactor_view(ringfactor.data(), {ntheta}, vector<ptrdiff_t>());
            
            // Perform synthesis
            synthesis_2d(alm_view, map_view, spin, lmax, mstart_view, 1, 
                        geometry, phi0, ringfactor_view, nthreads, mode);
            
            // Copy map from buffer to MATLAB
            float *map_out = (float *)mxGetData(map_arr);
            for (size_t imap = 0; imap < nmaps; ++imap) {
                for (size_t itheta = 0; itheta < ntheta; ++itheta) {
                    for (size_t iphi = 0; iphi < nphi; ++iphi) {
                        size_t idx_ducc = imap * ntheta * nphi + itheta * nphi + iphi; // Row-major
                        size_t idx_matlab = imap + itheta * nmaps + iphi * nmaps * ntheta; // Column-major
                        map_out[idx_matlab] = map_buffer[idx_ducc];
                    }
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis2D:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = map_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

