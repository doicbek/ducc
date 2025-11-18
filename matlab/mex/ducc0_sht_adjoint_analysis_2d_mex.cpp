/*
 * DUCC0 SHT Adjoint Analysis 2D MEX Interface
 * 
 * MATLAB MEX gateway for adjoint spherical harmonic analysis (alm2map) for 2D grids
 * This is the adjoint operation of analysis_2d.
 * 
 * Usage:
 *   map = ducc0_sht_adjoint_analysis_2d_mex(alm, lmax, spin, geometry, ntheta, nphi, mmax, phi0, ringfactor, nthreads)
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

// Get default ntheta for geometry
size_t get_default_ntheta(size_t lmax, const string &geometry)
{
    if (geometry == "CC") return lmax + 2;
    if (geometry == "DH") return 2 * lmax + 2;
    return lmax + 1;
}

// Get default nphi for geometry
size_t get_default_nphi(size_t mmax)
{
    return 2 * mmax + 2;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 4) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "At least 4 inputs required: alm, lmax, spin, geometry");
        }
        
        const mxArray *alm_arr = prhs[0];
        if (mxIsEmpty(alm_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "alm cannot be empty");
        }
        
        if (!mxIsComplex(alm_arr)) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "alm must be complex");
        }
        
        // Parse parameters
        size_t lmax = (size_t)mxGetScalar(prhs[1]);
        size_t spin = (size_t)mxGetScalar(prhs[2]);
        string geometry = getStringParam(prhs[3], "");
        size_t ntheta = getOptionalParam<size_t>(nrhs > 4 ? prhs[4] : nullptr, 0);
        size_t nphi = getOptionalParam<size_t>(nrhs > 5 ? prhs[5] : nullptr, 0);
        size_t mmax = getOptionalParam<size_t>(nrhs > 6 ? prhs[6] : nullptr, lmax);
        double phi0 = getOptionalParam<double>(nrhs > 7 ? prhs[7] : nullptr, 0.0);
        const mxArray *ringfactor_arr = (nrhs > 8) ? prhs[8] : nullptr;
        size_t nthreads = getOptionalParam<size_t>(nrhs > 9 ? prhs[9] : nullptr, 0);
        
        if (mmax > lmax) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "mmax must be <= lmax");
        }
        
        size_t nmaps = get_nmaps(spin);
        size_t ncomp = (spin == 0) ? 1 : 2;
        
        // Get alm dimensions
        mwSize ndim = mxGetNumberOfDimensions(alm_arr);
        const mwSize *dims = mxGetDimensions(alm_arr);
        
        size_t ncomp_in = 1;
        size_t nalm = 0;
        if (ndim == 1) {
            nalm = dims[0];
        } else if (ndim == 2) {
            ncomp_in = dims[0];
            nalm = dims[1];
        } else {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "alm must be 1D or 2D array");
        }
        
        if (ncomp_in != ncomp) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "alm first dimension must be ncomp");
        }
        
        // Build mstart array
        vector<size_t> mstart = build_mstart(lmax, mmax);
        size_t nalm_expected = ((mmax+1)*(mmax+2))/2 + (mmax+1)*(lmax-mmax);
        
        if (nalm != nalm_expected) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                "alm size does not match lmax and mmax");
        }
        
        // Determine ntheta and nphi if not provided
        if (ntheta == 0) {
            ntheta = get_default_ntheta(lmax, geometry);
        }
        if (nphi == 0) {
            nphi = get_default_nphi(mmax);
        }
        
        // Get ringfactor (default: all ones)
        vector<double> ringfactor(ntheta, 1.0);
        if (ringfactor_arr != nullptr && !mxIsEmpty(ringfactor_arr)) {
            size_t nring = mxGetNumberOfElements(ringfactor_arr);
            if (nring != ntheta) {
                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:InputError", 
                    "ringfactor size must match ntheta");
            }
            double *ringfactor_data = mxGetPr(ringfactor_arr);
            for (size_t i = 0; i < ntheta; ++i) {
                ringfactor[i] = ringfactor_data[i];
            }
        }
        
        // Determine data type
        mxClassID class_id = mxGetClassID(alm_arr);
        
        // Create output array (real)
        mwSize map_dims[3] = {nmaps, ntheta, nphi};
        mxArray *map_arr = mxCreateNumericArray(3, map_dims, class_id, mxREAL);
        
        // Process based on data type
        if (class_id == mxDOUBLE_CLASS) {
            // Complex double input -> real double output
            vector<complex<double>> alm_buffer;
            vector<double> map_buffer;
            
            // Prepare alm shape for DUCC (row-major): [ncomp, nalm]
            size_t alm_nelem = ncomp * nalm_expected;
            alm_buffer.resize(alm_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const double *real_data = mxGetPr(alm_arr);
            const double *imag_data = mxGetPi(alm_arr);
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
            
            // Prepare map buffer and view
            size_t map_nelem = nmaps * ntheta * nphi;
            map_buffer.resize(map_nelem);
            array<size_t,3> map_shape = {nmaps, ntheta, nphi};
            vmav<double,3> map_view(map_buffer.data(), map_shape);
            
            // Create mstart view
            array<size_t,1> mstart_shape = {mmax+1};
            cmav<size_t,1> mstart_view(mstart.data(), mstart_shape);
            
            // Create ringfactor view
            array<size_t,1> ringfactor_shape = {ntheta};
            cmav<double,1> ringfactor_view(ringfactor.data(), ringfactor_shape);
            
            // Perform adjoint analysis
            adjoint_analysis_2d(alm_view, map_view, spin, lmax, mstart_view, 1, 
                              geometry, phi0, ringfactor_view, nthreads);
            
            // Copy map from buffer to MATLAB
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
            size_t alm_nelem = ncomp * nalm_expected;
            alm_buffer.resize(alm_nelem);
            
            // Reorder from MATLAB column-major to DUCC row-major
            const float *real_data = (const float *)mxGetData(alm_arr);
            const float *imag_data = (const float *)mxGetImagData(alm_arr);
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
            
            cmav<complex<float>,2> alm_view(alm_buffer.data(), {ncomp, nalm_expected}, vector<ptrdiff_t>());
            
            // Prepare map buffer and view
            size_t map_nelem = nmaps * ntheta * nphi;
            map_buffer.resize(map_nelem);
            vector<size_t> map_shape = {nmaps, ntheta, nphi};
            vmav<float,3> map_view(map_buffer.data(), map_shape, vector<ptrdiff_t>());
            
            // Create mstart view
            cmav<size_t,1> mstart_view(mstart.data(), {mmax+1}, vector<ptrdiff_t>());
            
            // Create ringfactor view
            cmav<double,1> ringfactor_view(ringfactor.data(), {ntheta}, vector<ptrdiff_t>());
            
            // Perform adjoint analysis
            adjoint_analysis_2d(alm_view, map_view, spin, lmax, mstart_view, 1, 
                              geometry, phi0, ringfactor_view, nthreads);
            
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
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointAnalysis2D:TypeError", 
                "Unsupported data type. Only double and single precision are supported.");
        }
        
        plhs[0] = map_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

