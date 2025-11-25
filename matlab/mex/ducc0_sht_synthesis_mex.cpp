/*
 * DUCC0 SHT Synthesis MEX Interface
 * 
 * MATLAB MEX gateway for spherical harmonic synthesis (alm2map) for arbitrary theta rings
 * 
 * Usage:
 *   map = ducc0_sht_synthesis_mex(alm, lmax, spin, theta, nphi, phi0, ringstart, ...
 *                                  mmax, mstart, lstride, pixstride, ringfactor, mode, theta_interpol, nthreads)
 * 
 * Inputs:
 *   alm: Spherical harmonic coefficients (complex array, shape [ncomp, nalm] or [nalm])
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
 *   map: Synthesized map (real array, shape [nmaps, npix])
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

// Calculate minimum map dimension from ring parameters
size_t min_mapdim(const vector<size_t> &nphi, const vector<size_t> &ringstart, ptrdiff_t pixstride)
{
    size_t max_pix = 0;
    for (size_t i = 0; i < nphi.size(); ++i) {
        size_t ring_end = ringstart[i] + (nphi[i] - 1) * pixstride;
        if (ring_end > max_pix) max_pix = ring_end;
    }
    return max_pix + 1;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 7) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "At least 7 inputs required: alm, lmax, spin, theta, nphi, phi0, ringstart");
        }
        
        const mxArray *alm_arr = prhs[0];
        if (mxIsEmpty(alm_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "alm array cannot be empty");
        }
        
        if (!mxIsComplex(alm_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "alm must be complex");
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
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "theta must be a non-empty double array");
        }
        if (!mxIsNumeric(nphi_arr) || mxGetNumberOfElements(nphi_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "nphi must be a non-empty numeric array");
        }
        if (!mxIsDouble(phi0_arr) || mxGetNumberOfElements(phi0_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "phi0 must be a non-empty double array");
        }
        if (!mxIsNumeric(ringstart_arr) || mxGetNumberOfElements(ringstart_arr) == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "ringstart must be a non-empty numeric array");
        }
        
        size_t nrings = mxGetNumberOfElements(theta_arr);
        if (mxGetNumberOfElements(nphi_arr) != nrings ||
            mxGetNumberOfElements(phi0_arr) != nrings ||
            mxGetNumberOfElements(ringstart_arr) != nrings) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
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
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "mmax must be <= lmax");
        }
        
        SHT_mode mode = get_sht_mode(mode_str);
        size_t nmaps = get_nmaps(spin, mode);
        size_t ncomp = get_nalm(spin, mode);
        
        // Get alm dimensions
        mwSize ndim = mxGetNumberOfDimensions(alm_arr);
        const mwSize *dims = mxGetDimensions(alm_arr);
        
        size_t N = 1;  // Number of maps in batch
        size_t ncomp_in = 1;
        size_t nalm = 0;
        bool is_batch_mode = false;
        
        if (ndim == 1) {
            nalm = dims[0];
        } else if (ndim == 2) {
            ncomp_in = dims[0];
            nalm = dims[1];
        } else if (ndim == 3) {
            // Batch mode: [N, ncomp, nalm]
            is_batch_mode = true;
            N = dims[0];
            ncomp_in = dims[1];
            nalm = dims[2];
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "alm must be 1D, 2D array [ncomp, nalm], or 3D array [N, ncomp, nalm]");
        }
        
        // Build or get mstart
        vector<size_t> mstart;
        if (mstart_arr != nullptr && !mxIsEmpty(mstart_arr)) {
            size_t mstart_size = mxGetNumberOfElements(mstart_arr);
            if (mstart_size != mmax + 1) {
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
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
        if (nalm != nalm_expected) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "alm size does not match lmax and mmax");
        }
        
        if (ncomp_in != ncomp) {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                "alm first/second dimension must be ncomp");
        }
        
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
                mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:InputError", 
                    "ringfactor size must match nrings");
            }
            const double *rf_data = mxGetPr(ringfactor_arr);
            for (size_t i = 0; i < nrings; ++i) {
                ringfactor[i] = rf_data[i];
            }
        }
        
        // Calculate minimum map size
        size_t npix = min_mapdim(nphi, ringstart, pixstride);
        
        // Determine data type
        mxClassID class_id = mxGetClassID(alm_arr);
        
        // Create output array (real, shape [N, nmaps, npix] for batch, [nmaps, npix] for single)
        mxArray *map_arr;
        if (is_batch_mode) {
            mwSize map_dims[3] = {N, nmaps, npix};
            map_arr = mxCreateNumericArray(3, map_dims, class_id, mxREAL);
        } else {
            mwSize map_dims[2] = {nmaps, npix};
            map_arr = mxCreateNumericArray(2, map_dims, class_id, mxREAL);
        }
        
        // Create mstart view (reused for all maps)
        array<size_t,1> mstart_shape = {mmax+1};
        cmav<size_t,1> mstart_view(mstart.data(), mstart_shape);
        
        // Create ring parameter views (reused for all maps)
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
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            const double *real_data = mxGetPr(alm_arr);
            const double *imag_data = mxGetPi(alm_arr);
            double *map_out = mxGetPr(map_arr);
            
            if (is_batch_mode) {
                // Batch mode: use batch C++ function
                // Convert from MATLAB column-major to DUCC row-major for all maps
                vector<complex<double>> alm_buffer(N * ncomp * nalm_expected);
                for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                    for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                        for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                            // MATLAB column-major: [N, ncomp, nalm]
                            size_t idx_matlab = ibatch + icomp * N + ialm * N * ncomp;
                            // DUCC row-major: [N, ncomp, nalm]
                            size_t idx_ducc = ibatch * ncomp * nalm_expected + icomp * nalm_expected + ialm;
                            alm_buffer[idx_ducc] = complex<double>(real_data[idx_matlab], imag_data[idx_matlab]);
                        }
                    }
                }
                
                // Create 3D views for batch processing (row-major)
                array<size_t,3> alm_shape = {N, ncomp, nalm_expected};
                cmav<complex<double>,3> alm_view(alm_buffer.data(), alm_shape);
                
                vector<double> map_buffer(N * nmaps * npix);
                array<size_t,3> map_shape = {N, nmaps, npix};
                vmav<double,3> map_view(map_buffer.data(), map_shape);
                
                // Call batch function (reuses setup internally)
                synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                               theta_view, nphi_view, phi0_view, ringstart_view,
                               ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                
                // Copy from buffer to MATLAB (column-major)
                for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                    for (size_t imap = 0; imap < nmaps; ++imap) {
                        for (size_t ipix = 0; ipix < npix; ++ipix) {
                            size_t idx_buffer = ibatch * nmaps * npix + imap * npix + ipix;
                            size_t idx_matlab = ibatch + imap * N + ipix * N * nmaps;
                            map_out[idx_matlab] = map_buffer[idx_buffer];
                        }
                    }
                }
            } else {
                // Single mode: use regular function
                vector<complex<double>> alm_buffer(ncomp * nalm_expected);
                vector<double> map_buffer(nmaps * npix);
                
                // Reorder from MATLAB column-major to DUCC row-major
                if (ndim == 1) {
                    for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                        alm_buffer[ialm] = complex<double>(real_data[ialm], imag_data[ialm]);
                    }
                } else {
                    for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                        for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                            size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                            size_t idx_ducc = icomp * nalm_expected + ialm; // Row-major
                            alm_buffer[idx_ducc] = complex<double>(real_data[idx_matlab], imag_data[idx_matlab]);
                        }
                    }
                }
                
                array<size_t,2> alm_shape = {ncomp, nalm_expected};
                cmav<complex<double>,2> alm_view(alm_buffer.data(), alm_shape);
                
                array<size_t,2> map_shape = {nmaps, npix};
                vmav<double,2> map_view(map_buffer.data(), map_shape);
                
                // Perform synthesis
                synthesis(alm_view, map_view, spin, lmax, mstart_view, lstride,
                         theta_view, nphi_view, phi0_view, ringstart_view,
                         ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                
                // Copy map from buffer to MATLAB
                for (size_t imap = 0; imap < nmaps; ++imap) {
                    for (size_t ipix = 0; ipix < npix; ++ipix) {
                        size_t idx_ducc = imap * npix + ipix; // Row-major
                        size_t idx_matlab = imap + ipix * nmaps; // Column-major
                        map_out[idx_matlab] = map_buffer[idx_ducc];
                    }
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            const float *real_data = (const float *)mxGetData(alm_arr);
            const float *imag_data = (const float *)mxGetImagData(alm_arr);
            float *map_out = (float *)mxGetData(map_arr);
            
            if (is_batch_mode) {
                // Batch mode: use batch C++ function
                // Convert from MATLAB column-major to DUCC row-major for all maps
                vector<complex<float>> alm_buffer(N * ncomp * nalm_expected);
                for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                    for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                        for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                            // MATLAB column-major: [N, ncomp, nalm]
                            size_t idx_matlab = ibatch + icomp * N + ialm * N * ncomp;
                            // DUCC row-major: [N, ncomp, nalm]
                            size_t idx_ducc = ibatch * ncomp * nalm_expected + icomp * nalm_expected + ialm;
                            alm_buffer[idx_ducc] = complex<float>(real_data[idx_matlab], imag_data[idx_matlab]);
                        }
                    }
                }
                
                // Create 3D views for batch processing (row-major)
                array<size_t,3> alm_shape = {N, ncomp, nalm_expected};
                cmav<complex<float>,3> alm_view(alm_buffer.data(), alm_shape);
                
                vector<float> map_buffer(N * nmaps * npix);
                array<size_t,3> map_shape = {N, nmaps, npix};
                vmav<float,3> map_view(map_buffer.data(), map_shape);
                
                // Call batch function (reuses setup internally)
                synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                               theta_view, nphi_view, phi0_view, ringstart_view,
                               ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                
                // Copy from buffer to MATLAB (column-major)
                for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                    for (size_t imap = 0; imap < nmaps; ++imap) {
                        for (size_t ipix = 0; ipix < npix; ++ipix) {
                            size_t idx_buffer = ibatch * nmaps * npix + imap * npix + ipix;
                            size_t idx_matlab = ibatch + imap * N + ipix * N * nmaps;
                            map_out[idx_matlab] = map_buffer[idx_buffer];
                        }
                    }
                }
            } else {
                // Single mode: use regular function
                vector<complex<float>> alm_buffer(ncomp * nalm_expected);
                vector<float> map_buffer(nmaps * npix);
                
                // Reorder from MATLAB column-major to DUCC row-major
                if (ndim == 1) {
                    for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                        alm_buffer[ialm] = complex<float>(real_data[ialm], imag_data[ialm]);
                    }
                } else {
                    for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                        for (size_t ialm = 0; ialm < nalm_expected; ++ialm) {
                            size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                            size_t idx_ducc = icomp * nalm_expected + ialm; // Row-major
                            alm_buffer[idx_ducc] = complex<float>(real_data[idx_matlab], imag_data[idx_matlab]);
                        }
                    }
                }
                
                array<size_t,2> alm_shape = {ncomp, nalm_expected};
                cmav<complex<float>,2> alm_view(alm_buffer.data(), alm_shape);
                
                array<size_t,2> map_shape = {nmaps, npix};
                vmav<float,2> map_view(map_buffer.data(), map_shape);
                
                // Perform synthesis
                synthesis(alm_view, map_view, spin, lmax, mstart_view, lstride,
                         theta_view, nphi_view, phi0_view, ringstart_view,
                         ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                
                // Copy map from buffer to MATLAB
                for (size_t imap = 0; imap < nmaps; ++imap) {
                    for (size_t ipix = 0; ipix < npix; ++ipix) {
                        size_t idx_ducc = imap * npix + ipix; // Row-major
                        size_t idx_matlab = imap + ipix * nmaps; // Column-major
                        map_out[idx_matlab] = map_buffer[idx_ducc];
                    }
                }
            }
            
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:Synthesis:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = map_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

