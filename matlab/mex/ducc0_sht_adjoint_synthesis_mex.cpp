/*
 * DUCC0 SHT Adjoint Synthesis MEX Interface
 * 
 * MATLAB MEX gateway for adjoint spherical harmonic synthesis (map2alm) for arbitrary theta rings
 * This is the adjoint operation of synthesis.
 * 
 * Usage:
 *   alm = ducc0_sht_adjoint_synthesis_mex(map, lmax, spin, theta, nphi, phi0, ringstart, ...
 *                                         mmax, mstart, lstride, pixstride, ringfactor, mode, theta_interpol, nthreads)
 * 
 * Inputs:
 *   map: Map data (real array, shape [nmaps, npix])
 *   lmax: Maximum multipole order l
 *   spin: Spin value (0, 1, or 2)
 *   theta: Colatitudes of map rings (double array, size [nrings])
 *   nphi: Number of pixels per ring (size_t array, size [nrings])
 *   phi0: Azimuth of first pixel in each ring (double array, size [nrings])
 *   ringstart: Index in map where first pixel of each ring is stored (size_t array, size [nrings])
 *   mmax: Maximum m order (optional, default: lmax)
 *   mstart: mstart array (optional, default: computed from lmax, mmax)
 *   lstride: Stride in alm between l and l+1 (optional, default: 1)
 *   pixstride: Stride in map between pixels (optional, default: 1)
 *   ringfactor: Ring factors (optional, size [nrings], default: all ones)
 *   mode: Transform mode ('STANDARD', 'GRAD_ONLY', 'DERIV1') (optional, default: 'STANDARD')
 *   theta_interpol: Use theta interpolation (optional, default: false)
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
#include <algorithm>

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

// Calculate minimum alm dimension from lmax, mstart, lstride
size_t min_almdim(size_t lmax, const vector<size_t> &mstart, ptrdiff_t lstride)
{
    size_t res = 0;
    for (size_t i = 0; i < mstart.size(); ++i) {
        ptrdiff_t ifirst = ptrdiff_t(mstart[i]) + ptrdiff_t(i) * lstride;
        if (ifirst < 0) {
            MR_fail("impossible a_lm memory layout");
        }
        ptrdiff_t ilast = ptrdiff_t(mstart[i]) + ptrdiff_t(lmax) * lstride;
        if (ilast < 0) {
            MR_fail("impossible a_lm memory layout");
        }
        size_t max_idx = size_t(std::max(ifirst, ilast));
        if (max_idx > res) res = max_idx;
    }
    return res + 1;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 7) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "At least 7 inputs required: map, lmax, spin, theta, nphi, phi0, ringstart");
        }
        
        const mxArray *map_arr = prhs[0];
        if (mxIsEmpty(map_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "map array cannot be empty");
        }
        
        if (mxIsComplex(map_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "map must be real");
        }
        
        // Parse required parameters
        size_t lmax = (size_t)mxGetScalar(prhs[1]);
        size_t spin = (size_t)mxGetScalar(prhs[2]);
        
        // Get ring parameters
        const mxArray *theta_arr = prhs[3];
        const mxArray *nphi_arr = prhs[4];
        const mxArray *phi0_arr = prhs[5];
        const mxArray *ringstart_arr = prhs[6];
        
        if (!mxIsDouble(theta_arr) || mxGetNumberOfElements(theta_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "theta must be a non-empty double array");
        }
        if (!mxIsNumeric(nphi_arr) || mxGetNumberOfElements(nphi_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "nphi must be a non-empty numeric array");
        }
        if (!mxIsDouble(phi0_arr) || mxGetNumberOfElements(phi0_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "phi0 must be a non-empty double array");
        }
        if (!mxIsNumeric(ringstart_arr) || mxGetNumberOfElements(ringstart_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "ringstart must be a non-empty numeric array");
        }
        
        size_t nrings = mxGetNumberOfElements(theta_arr);
        if (mxGetNumberOfElements(nphi_arr) != nrings ||
            mxGetNumberOfElements(phi0_arr) != nrings ||
            mxGetNumberOfElements(ringstart_arr) != nrings) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "theta, nphi, phi0, and ringstart must have the same size");
        }
        
        // Parse optional parameters
        size_t mmax = getOptionalParam<size_t>(nrhs > 7 ? prhs[7] : nullptr, lmax);
        const mxArray *mstart_arr = (nrhs > 8) ? prhs[8] : nullptr;
        ptrdiff_t lstride = getOptionalParam<ptrdiff_t>(nrhs > 9 ? prhs[9] : nullptr, 1);
        ptrdiff_t pixstride = getOptionalParam<ptrdiff_t>(nrhs > 10 ? prhs[10] : nullptr, 1);
        const mxArray *ringfactor_arr = (nrhs > 11) ? prhs[11] : nullptr;
        string mode_str = getStringParam(nrhs > 12 ? prhs[12] : nullptr, "STANDARD");
        bool theta_interpol = getOptionalParam<bool>(nrhs > 13 ? prhs[13] : nullptr, false);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 14 ? prhs[14] : nullptr, 0);
        
        if (mmax > lmax) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "mmax must be <= lmax");
        }
        
        SHT_mode mode = get_sht_mode(mode_str);
        size_t nmaps = get_nmaps(spin, mode);
        size_t ncomp = get_nalm(spin, mode);
        
        // Get map dimensions
        mwSize ndim = mxGetNumberOfDimensions(map_arr);
        const mwSize *dims = mxGetDimensions(map_arr);
        
        if (ndim != 2) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "map must be 2D array [nmaps, npix]");
        }
        
        size_t nmaps_in = dims[0];
        size_t npix = dims[1];
        
        if (nmaps_in != nmaps) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                "map first dimension must be nmaps");
        }
        
        // Build or get mstart
        vector<size_t> mstart;
        if (mstart_arr != nullptr && !mxIsEmpty(mstart_arr)) {
            size_t mstart_size = mxGetNumberOfElements(mstart_arr);
            if (mstart_size != mmax + 1) {
                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                    "mstart size must be mmax+1");
            }
            mstart.resize(mstart_size);
            const double *mstart_data = mxGetPr(mstart_arr);
            for (size_t i = 0; i < mstart_size; ++i) {
                mstart[i] = (size_t)mstart_data[i];
            }
        } else {
            mstart = build_mstart(lmax, mmax);
        }
        
        size_t nalm_expected = ((mmax+1)*(mmax+2))/2 + (mmax+1)*(lmax-mmax);
        size_t nalm_dim = min_almdim(lmax, mstart, lstride);
        
        // Get ring parameter arrays
        vector<double> theta(nrings);
        vector<size_t> nphi(nrings);
        vector<double> phi0(nrings);
        vector<size_t> ringstart(nrings);
        
        const double *theta_data = mxGetPr(theta_arr);
        const double *nphi_data = mxGetPr(nphi_arr);
        const double *phi0_data = mxGetPr(phi0_arr);
        const double *ringstart_data = mxGetPr(ringstart_arr);
        
        for (size_t i = 0; i < nrings; ++i) {
            theta[i] = theta_data[i];
            nphi[i] = (size_t)nphi_data[i];
            phi0[i] = phi0_data[i];
            ringstart[i] = (size_t)ringstart_data[i];
        }
        
        // Get ringfactor (default: all ones)
        vector<double> ringfactor(nrings, 1.0);
        if (ringfactor_arr != nullptr && !mxIsEmpty(ringfactor_arr)) {
            size_t rf_size = mxGetNumberOfElements(ringfactor_arr);
            if (rf_size != nrings) {
                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                    "ringfactor size must match nrings");
            }
            const double *rf_data = mxGetPr(ringfactor_arr);
            for (size_t i = 0; i < nrings; ++i) {
                ringfactor[i] = rf_data[i];
            }
        }
        
        // Determine data type
        mxClassID class_id = mxGetClassID(map_arr);
        
        // Create output array (complex, shape [ncomp, nalm_dim])
        mwSize alm_dims[2] = {ncomp, nalm_dim};
        mxArray *alm_arr = mxCreateNumericArray(2, alm_dims, class_id, mxCOMPLEX);
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Real double input -> complex double output
            vector<double> map_buffer;
            vector<complex<double>> alm_buffer;
            
            // Prepare map shape for DUCC (row-major): [nmaps, npix]
            size_t map_nelem = nmaps * npix;
            map_buffer.resize(map_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const double *map_data = mxGetPr(map_arr);
            for (size_t imap = 0; imap < nmaps; ++imap) {
                for (size_t ipix = 0; ipix < npix; ++ipix) {
                    size_t idx_matlab = imap + ipix * nmaps; // Column-major
                    size_t idx_ducc = imap * npix + ipix; // Row-major
                    map_buffer[idx_ducc] = map_data[idx_matlab];
                }
            }
            
            array<size_t,2> map_shape = {nmaps, npix};
            cmav<double,2> map_view(map_buffer.data(), map_shape);
            
            // Prepare alm buffer and view
            size_t alm_nelem = ncomp * nalm_dim;
            alm_buffer.resize(alm_nelem);
            array<size_t,2> alm_shape = {ncomp, nalm_dim};
            vmav<complex<double>,2> alm_view(alm_buffer.data(), alm_shape);
            
            // Create mstart view
            array<size_t,1> mstart_shape = {mmax+1};
            cmav<size_t,1> mstart_view(mstart.data(), mstart_shape);
            
            // Create ring parameter views
            array<size_t,1> theta_shape = {nrings};
            cmav<double,1> theta_view(theta.data(), theta_shape);
            array<size_t,1> nphi_shape = {nrings};
            cmav<size_t,1> nphi_view(nphi.data(), nphi_shape);
            array<size_t,1> phi0_shape = {nrings};
            cmav<double,1> phi0_view(phi0.data(), phi0_shape);
            array<size_t,1> ringstart_shape = {nrings};
            cmav<size_t,1> ringstart_view(ringstart.data(), ringstart_shape);
            array<size_t,1> ringfactor_shape = {nrings};
            cmav<double,1> ringfactor_view(ringfactor.data(), ringfactor_shape);
            
            // Perform adjoint synthesis
            adjoint_synthesis(alm_view, map_view, spin, lmax, mstart_view, lstride,
                             theta_view, nphi_view, phi0_view, ringstart_view,
                             ringfactor_view, pixstride, nthreads, mode, theta_interpol);
            
            // Copy alm from buffer to MATLAB
            double *alm_real = mxGetPr(alm_arr);
            double *alm_imag = mxGetPi(alm_arr);
            for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                    size_t idx_ducc = icomp * nalm_dim + ialm; // Row-major
                    size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                    alm_real[idx_matlab] = alm_buffer[idx_ducc].real();
                    alm_imag[idx_matlab] = alm_buffer[idx_ducc].imag();
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            // Real single input -> complex single output
            vector<float> map_buffer;
            vector<complex<float>> alm_buffer;
            
            // Prepare map shape for DUCC (row-major): [nmaps, npix]
            size_t map_nelem = nmaps * npix;
            map_buffer.resize(map_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const float *map_data = (const float *)mxGetData(map_arr);
            for (size_t imap = 0; imap < nmaps; ++imap) {
                for (size_t ipix = 0; ipix < npix; ++ipix) {
                    size_t idx_matlab = imap + ipix * nmaps; // Column-major
                    size_t idx_ducc = imap * npix + ipix; // Row-major
                    map_buffer[idx_ducc] = map_data[idx_matlab];
                }
            }
            
            array<size_t,2> map_shape = {nmaps, npix};
            cmav<float,2> map_view(map_buffer.data(), map_shape);
            
            // Prepare alm buffer and view
            size_t alm_nelem = ncomp * nalm_dim;
            alm_buffer.resize(alm_nelem);
            array<size_t,2> alm_shape = {ncomp, nalm_dim};
            vmav<complex<float>,2> alm_view(alm_buffer.data(), alm_shape);
            
            // Create mstart view
            array<size_t,1> mstart_shape = {mmax+1};
            cmav<size_t,1> mstart_view(mstart.data(), mstart_shape);
            
            // Create ring parameter views
            array<size_t,1> theta_shape = {nrings};
            cmav<double,1> theta_view(theta.data(), theta_shape);
            array<size_t,1> nphi_shape = {nrings};
            cmav<size_t,1> nphi_view(nphi.data(), nphi_shape);
            array<size_t,1> phi0_shape = {nrings};
            cmav<double,1> phi0_view(phi0.data(), phi0_shape);
            array<size_t,1> ringstart_shape = {nrings};
            cmav<size_t,1> ringstart_view(ringstart.data(), ringstart_shape);
            array<size_t,1> ringfactor_shape = {nrings};
            cmav<double,1> ringfactor_view(ringfactor.data(), ringfactor_shape);
            
            // Perform adjoint synthesis
            adjoint_synthesis(alm_view, map_view, spin, lmax, mstart_view, lstride,
                             theta_view, nphi_view, phi0_view, ringstart_view,
                             ringfactor_view, pixstride, nthreads, mode, theta_interpol);
            
            // Copy alm from buffer to MATLAB
            float *alm_real = (float *)mxGetData(alm_arr);
            float *alm_imag = (float *)mxGetImagData(alm_arr);
            for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                    size_t idx_ducc = icomp * nalm_dim + ialm; // Row-major
                    size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                    alm_real[idx_matlab] = alm_buffer[idx_ducc].real();
                    alm_imag[idx_matlab] = alm_buffer[idx_ducc].imag();
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = alm_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

