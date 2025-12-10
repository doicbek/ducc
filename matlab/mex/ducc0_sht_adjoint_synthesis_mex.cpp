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
 *   map: Map data (real array, dense or sparse, shape [nmaps, npix] or [N, nmaps, npix])
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
#include <cstdint>
#include <limits>

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
        
        // Check if sparse (needed for dimension parsing)
        bool is_sparse = mxIsSparse(map_arr);
        
        // Get map dimensions
        // Note: Sparse arrays in MATLAB are always 2D, even if conceptually 3D
        // When a 3D sparse array is passed, it appears as 2D with size (N*nmaps) x npix
        mwSize ndim = mxGetNumberOfDimensions(map_arr);
        const mwSize *dims = mxGetDimensions(map_arr);
        
        size_t N = 1;  // Number of maps in batch
        size_t nmaps_in = 0;
        size_t npix = 0;
        bool is_batch_mode = false;
        
        if (is_sparse) {
            // For sparse arrays, MATLAB always presents them as 2D
            // For map2alm, sparse arrays come as [N, ncomp*npix] for batch mode
            // or [ncomp, npix] for single map mode
            // Infer format from dimensions and check if divisible by nmaps
            if (dims[0] > 1 && dims[1] >= nmaps) {
                // Likely batch mode: [N, ncomp*npix] where N = dims[0]
                // npix_total = ncomp * npix, so npix = npix_total / ncomp
                size_t npix_total = dims[1];
                if (npix_total % nmaps != 0) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                        "sparse map second dimension must be divisible by nmaps for batch mode");
                }
                is_batch_mode = true;
                N = dims[0];
                npix = npix_total / nmaps;
                nmaps_in = nmaps;
            } else if (dims[0] == nmaps) {
                // Single map mode: [nmaps, npix]
                nmaps_in = dims[0];
                npix = dims[1];
                if (nmaps_in != nmaps) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                        "map first dimension must be nmaps");
                }
            } else {
                // Unknown format - try single map mode
                nmaps_in = dims[0];
                npix = dims[1];
                if (nmaps_in != nmaps) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                        "map dimensions do not match expected format for sparse array");
                }
            }
        } else {
            // Dense arrays can be 2D or 3D
            if (ndim == 2) {
                // Single map mode: [nmaps, npix]
                nmaps_in = dims[0];
                npix = dims[1];
                if (nmaps_in != nmaps) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                        "map first dimension must be nmaps");
                }
            } else if (ndim == 3) {
                // Batch mode: [N, ncomp, npix]
                is_batch_mode = true;
                N = dims[0];
                nmaps_in = dims[1];
                npix = dims[2];
                if (nmaps_in != nmaps) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                        "map second dimension must be nmaps");
                }
            } else {
                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:InputError", 
                    "map must be 2D array [nmaps, npix] or 3D array [N, nmaps, npix]");
            }
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
        
        // Validate array sizes to prevent overflow and excessive memory allocation
        // Check for potential overflow in size calculations
        const size_t MAX_SAFE_SIZE = std::numeric_limits<size_t>::max() / (2 * sizeof(double)); // Conservative limit
        if (npix > MAX_SAFE_SIZE) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                "npix (%zu) exceeds maximum safe size (%zu). Input map may be too large.", 
                npix, MAX_SAFE_SIZE);
        }
        if (nalm_dim > MAX_SAFE_SIZE) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                "nalm_dim (%zu) exceeds maximum safe size (%zu). lmax (%zu) may be too large.", 
                nalm_dim, MAX_SAFE_SIZE, lmax);
        }
        if (N > MAX_SAFE_SIZE) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                "N (%zu) exceeds maximum safe size (%zu). Batch size may be too large.", 
                N, MAX_SAFE_SIZE);
        }
        
        // Check output array size before allocation
        size_t output_size = N * ncomp * nalm_dim;
        if (output_size > MAX_SAFE_SIZE) {
            mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                "Output array size (%zu = %zu * %zu * %zu) exceeds maximum safe size (%zu). "
                "Consider reducing lmax, batch size, or processing in chunks.", 
                output_size, N, ncomp, nalm_dim, MAX_SAFE_SIZE);
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
        
        // For sparse arrays, we'll identify non-zero maps first and create output later
        // For dense arrays, create output array now
        mxArray *alm_arr = nullptr;
        vector<size_t> non_zero_row_indices;  // For sparse batch mode
        size_t N_nonzero = 0;  // For sparse batch mode
        
        if (is_batch_mode && is_sparse) {
            // Defer output array creation until we know how many non-zero maps there are
            // We'll identify non-zero rows first
        } else {
            // Create output array now for dense arrays or single map mode
            if (is_batch_mode) {
                mwSize alm_dims[3] = {N, ncomp, nalm_dim};
                alm_arr = mxCreateNumericArray(3, alm_dims, class_id, mxCOMPLEX);
                if (alm_arr == nullptr) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                        "Failed to create output array: dimensions [%zu, %zu, %zu]. "
                        "Not enough memory available. Consider reducing lmax or batch size.",
                        N, ncomp, nalm_dim);
                }
            } else {
                mwSize alm_dims[2] = {ncomp, nalm_dim};
                alm_arr = mxCreateNumericArray(2, alm_dims, class_id, mxCOMPLEX);
                if (alm_arr == nullptr) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                        "Failed to create output array: dimensions [%zu, %zu]. "
                        "Not enough memory available. Consider reducing lmax (currently %zu).",
                        ncomp, nalm_dim, lmax);
                }
            }
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
            // For sparse batch mode, we'll create alm_arr after identifying non-zero maps
            // For other cases, alm_arr is already created above
            double *alm_real = nullptr;
            double *alm_imag = nullptr;
            if (alm_arr != nullptr) {
                alm_real = mxGetPr(alm_arr);
                alm_imag = mxGetPi(alm_arr);
            }
            
            // Helper function to extract sparse or dense data into dense buffer
            auto extract_map_data = [&](vector<double> &map_buffer) {
                if (is_sparse) {
                    // Handle sparse array: use CSC format (Compressed Sparse Column)
                    // Initialize buffer to zeros (sparse arrays have implicit zeros)
                    fill(map_buffer.begin(), map_buffer.end(), 0.0);
                    
                    mwIndex *ir = mxGetIr(map_arr);  // Row indices
                    mwIndex *jc = mxGetJc(map_arr);  // Column pointers
                    const double *pr = mxGetPr(map_arr);  // Non-zero values
                    mwIndex nzmax = mxGetNzmax(map_arr);
                    
                    // For batch mode [N, nmaps, npix], sparse format is column-major
                    // Each column corresponds to one pixel position
                    // We need to map from MATLAB sparse indexing to our dense buffer layout
                    if (is_batch_mode) {
                        // For [N, nmaps, npix] in column-major: row index determines (ibatch, imap)
                        // column index determines ipix
                        for (mwIndex col = 0; col < npix; ++col) {
                            mwIndex row_start = jc[col];
                            mwIndex row_end = jc[col + 1];
                            for (mwIndex i = row_start; i < row_end; ++i) {
                                mwIndex row = ir[i];
                                double val = pr[i];
                                // Decompose row index: row = ibatch + imap * N
                                size_t ibatch = row % N;
                                size_t imap = row / N;
                                if (imap < nmaps) {
                                    // Convert to DUCC row-major: [N, nmaps, npix]
                                    size_t idx_ducc = ibatch * nmaps * npix + imap * npix + col;
                                    map_buffer[idx_ducc] = val;
                                }
                            }
                        }
                    } else {
                        // For [nmaps, npix]: row index is imap, column index is ipix
                        for (mwIndex col = 0; col < npix; ++col) {
                            mwIndex row_start = jc[col];
                            mwIndex row_end = jc[col + 1];
                            for (mwIndex i = row_start; i < row_end; ++i) {
                                mwIndex row = ir[i];
                                double val = pr[i];
                                if (row < nmaps) {
                                    // Convert to DUCC row-major: [nmaps, npix]
                                    size_t idx_ducc = row * npix + col;
                                    map_buffer[idx_ducc] = val;
                                }
                            }
                        }
                    }
                } else {
                    // Dense array: use existing conversion logic
                    const double *map_data = mxGetPr(map_arr);
                    if (is_batch_mode) {
                        // Convert from MATLAB column-major to DUCC row-major for all maps
                        for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                            for (size_t imap = 0; imap < nmaps; ++imap) {
                                for (size_t ipix = 0; ipix < npix; ++ipix) {
                                    // MATLAB column-major: [N, nmaps, npix]
                                    size_t idx_matlab = ibatch + imap * N + ipix * N * nmaps;
                                    // DUCC row-major: [N, nmaps, npix]
                                    size_t idx_ducc = ibatch * nmaps * npix + imap * npix + ipix;
                                    map_buffer[idx_ducc] = map_data[idx_matlab];
                                }
                            }
                        }
                    } else {
                        // Single map: [nmaps, npix]
                        for (size_t imap = 0; imap < nmaps; ++imap) {
                            for (size_t ipix = 0; ipix < npix; ++ipix) {
                                size_t idx_matlab = imap + ipix * nmaps; // Column-major
                                size_t idx_ducc = imap * npix + ipix; // Row-major
                                map_buffer[idx_ducc] = map_data[idx_matlab];
                            }
                        }
                    }
                }
            };
            
            if (is_batch_mode) {
                // For sparse arrays in batch mode, identify non-zero maps and process them together
                // to reuse computation (FFTs, basis functions computed once, reused for all maps)
                if (is_sparse) {
                    // Create output array first
                    mwSize alm_dims[3] = {N, ncomp, nalm_dim};
                    alm_arr = mxCreateNumericArray(3, alm_dims, class_id, mxCOMPLEX);
                    if (alm_arr == nullptr) {
                        mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                            "Failed to create output array: dimensions [%zu, %zu, %zu]. "
                            "Not enough memory available. Consider reducing lmax or batch size.",
                            N, ncomp, nalm_dim);
                    }
                    alm_real = mxGetPr(alm_arr);
                    alm_imag = mxGetPi(alm_arr);
                    
                    // Get sparse array structure
                    mwIndex *ir = mxGetIr(map_arr);  // Row indices
                    mwIndex *jc = mxGetJc(map_arr);  // Column pointers
                    const double *pr = mxGetPr(map_arr);  // Non-zero values
                    size_t npix_total = nmaps * npix;  // Total columns in sparse array
                    
                    // Find which rows (maps) have non-zero elements
                    vector<bool> row_has_data(N, false);
                    vector<size_t> non_zero_row_indices;
                    non_zero_row_indices.reserve(N);  // Reserve space, but likely much fewer
                    
                    for (mwIndex col = 0; col < npix_total; ++col) {
                        mwIndex row_start = jc[col];
                        mwIndex row_end = jc[col + 1];
                        for (mwIndex i = row_start; i < row_end; ++i) {
                            mwIndex row = ir[i];
                            if (row < (mwIndex)N && !row_has_data[row]) {
                                row_has_data[row] = true;
                                non_zero_row_indices.push_back(row);
                            }
                        }
                    }
                    
                    // Sort row indices for efficient processing
                    sort(non_zero_row_indices.begin(), non_zero_row_indices.end());
                    size_t N_nonzero = non_zero_row_indices.size();
                    
                    if (N_nonzero == 0) {
                        // All maps are zero - output array is already initialized to zeros
                    } else {
                        // Try to process all non-zero maps together first for maximum computation reuse
                        // Only fall back to chunking if memory allocation fails
                        vector<double> map_buffer;
                        bool use_chunking = false;
                        size_t chunk_size = 0;
                        
                        try {
                            // Try to allocate for all non-zero maps at once
                            map_buffer.resize(N_nonzero * nmaps * npix, 0.0);
                        } catch (const std::bad_alloc &e) {
                            // If allocation fails, fall back to chunking
                            use_chunking = true;
                            // Start with a reasonable chunk size (try larger first for better parallelism)
                            chunk_size = 1024;  // Process up to 1024 maps at a time
                            if (chunk_size > N_nonzero) chunk_size = N_nonzero;
                        }
                        
                        if (!use_chunking) {
                            // Extract sparse data for all non-zero maps
                            for (mwIndex col = 0; col < npix_total; ++col) {
                                mwIndex row_start = jc[col];
                                mwIndex row_end = jc[col + 1];
                                for (mwIndex i = row_start; i < row_end; ++i) {
                                    mwIndex row = ir[i];
                                    double val = pr[i];
                                    
                                    // Find position in non-zero batch
                                    auto it = lower_bound(non_zero_row_indices.begin(), non_zero_row_indices.end(), row);
                                    if (it != non_zero_row_indices.end() && *it == row) {
                                        size_t batch_idx = distance(non_zero_row_indices.begin(), it);
                                        
                                        // Column j represents pixel in flattened format
                                        // Map to [nmaps, npix]: component = j/npix, pixel = j%npix
                                        size_t imap = col / npix;
                                        size_t ipix = col % npix;
                                        if (imap < nmaps && ipix < npix) {
                                            // Convert to batch buffer: [N_nonzero, nmaps, npix]
                                            size_t idx_buffer = batch_idx * nmaps * npix + imap * npix + ipix;
                                            map_buffer[idx_buffer] = val;
                                        }
                                    }
                                }
                            }
                            
                            // Process all non-zero maps together using batch function
                            // This maximizes computation reuse and multithreading
                            array<size_t,3> map_shape = {N_nonzero, nmaps, npix};
                            cmav<double,3> map_view(map_buffer.data(), map_shape);
                            
                            vector<complex<double>> alm_buffer;
                            try {
                                alm_buffer.resize(N_nonzero * ncomp * nalm_dim);
                            } catch (const std::bad_alloc &e) {
                                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                                    "Failed to allocate alm_buffer: size = %zu * %zu * %zu = %zu elements. "
                                    "Not enough memory available. Consider reducing lmax.",
                                    N_nonzero, ncomp, nalm_dim, N_nonzero * ncomp * nalm_dim);
                            }
                            array<size_t,3> alm_shape = {N_nonzero, ncomp, nalm_dim};
                            vmav<complex<double>,3> alm_view(alm_buffer.data(), alm_shape);
                            
                            // Call batch function for all non-zero maps (maximizes computation reuse and multithreading)
                            adjoint_synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                                   theta_view, nphi_view, phi0_view, ringstart_view,
                                                   ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                            
                            // Copy results to output array
                            for (size_t i = 0; i < N_nonzero; ++i) {
                                size_t orig_row = non_zero_row_indices[i];
                                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                                    for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                                        size_t idx_buffer = i * ncomp * nalm_dim + icomp * nalm_dim + ialm;
                                        size_t idx_matlab = orig_row + icomp * N + ialm * N * ncomp;
                                        alm_real[idx_matlab] = alm_buffer[idx_buffer].real();
                                        alm_imag[idx_matlab] = alm_buffer[idx_buffer].imag();
                                    }
                                }
                            }
                        } else {
                            // Fall back to chunking if memory is limited
                            // Process non-zero maps in chunks
                            for (size_t chunk_start = 0; chunk_start < N_nonzero; ) {
                                size_t chunk_end = (chunk_start + chunk_size < N_nonzero) ? chunk_start + chunk_size : N_nonzero;
                                size_t chunk_N = chunk_end - chunk_start;
                                
                                // Allocate buffer for this chunk of non-zero maps
                                try {
                                    map_buffer.resize(chunk_N * nmaps * npix, 0.0);
                                } catch (const std::bad_alloc &e) {
                                    // If chunk allocation fails, try smaller chunk
                                    if (chunk_N > 1) {
                                        chunk_size = chunk_N / 2;
                                        continue;  // Retry this chunk with smaller size
                                    } else {
                                        mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                                            "Failed to allocate map_buffer: size = %zu * %zu * %zu = %zu elements. "
                                            "Not enough memory available. Consider processing fewer maps or reducing npix.",
                                            chunk_N, nmaps, npix, chunk_N * nmaps * npix);
                                    }
                                }
                            
                            // Extract sparse data only for this chunk of non-zero maps
                            for (mwIndex col = 0; col < npix_total; ++col) {
                                mwIndex row_start = jc[col];
                                mwIndex row_end = jc[col + 1];
                                for (mwIndex i = row_start; i < row_end; ++i) {
                                    mwIndex row = ir[i];
                                    double val = pr[i];
                                    
                                    // Find position in non-zero batch
                                    auto it = lower_bound(non_zero_row_indices.begin(), non_zero_row_indices.end(), row);
                                    if (it != non_zero_row_indices.end() && *it == row) {
                                        size_t batch_idx = distance(non_zero_row_indices.begin(), it);
                                        
                                        // Check if this map is in the current chunk
                                        if (batch_idx >= chunk_start && batch_idx < chunk_end) {
                                            size_t chunk_idx = batch_idx - chunk_start;
                                            
                                            // Column j represents pixel in flattened format
                                            // Map to [nmaps, npix]: component = j/npix, pixel = j%npix
                                            size_t imap = col / npix;
                                            size_t ipix = col % npix;
                                            if (imap < nmaps && ipix < npix) {
                                                // Convert to chunk buffer: [chunk_N, nmaps, npix]
                                                size_t idx_buffer = chunk_idx * nmaps * npix + imap * npix + ipix;
                                                map_buffer[idx_buffer] = val;
                                            }
                                        }
                                    }
                                }
                            }
                            
                            // Process this chunk using batch function (reuses computation within chunk)
                            array<size_t,3> map_shape = {chunk_N, nmaps, npix};
                            cmav<double,3> map_view(map_buffer.data(), map_shape);
                            
                            vector<complex<double>> alm_buffer;
                            try {
                                alm_buffer.resize(chunk_N * ncomp * nalm_dim);
                            } catch (const std::bad_alloc &e) {
                                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                                    "Failed to allocate alm_buffer: size = %zu * %zu * %zu = %zu elements. "
                                    "Not enough memory available. Consider reducing lmax.",
                                    chunk_N, ncomp, nalm_dim, chunk_N * ncomp * nalm_dim);
                            }
                            array<size_t,3> alm_shape = {chunk_N, ncomp, nalm_dim};
                            vmav<complex<double>,3> alm_view(alm_buffer.data(), alm_shape);
                            
                            // Call batch function for this chunk (reuses computation, enables multithreading)
                            adjoint_synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                                   theta_view, nphi_view, phi0_view, ringstart_view,
                                                   ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                            
                            // Copy results to output array, mapping chunk indices to original row indices
                            for (size_t i = 0; i < chunk_N; ++i) {
                                size_t orig_row = non_zero_row_indices[chunk_start + i];
                                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                                    for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                                        size_t idx_buffer = i * ncomp * nalm_dim + icomp * nalm_dim + ialm;
                                        size_t idx_matlab = orig_row + icomp * N + ialm * N * ncomp;
                                        alm_real[idx_matlab] = alm_buffer[idx_buffer].real();
                                        alm_imag[idx_matlab] = alm_buffer[idx_buffer].imag();
                                    }
                                }
                            }
                            
                            // Move to next chunk
                            chunk_start = chunk_end;
                        }
                        }
                    }
                } else {
                    // Dense batch mode: use batch C++ function
                    // Convert from MATLAB column-major to DUCC row-major for all maps
                    vector<double> map_buffer;
                    try {
                        map_buffer.resize(N * nmaps * npix);
                    } catch (const std::bad_alloc &e) {
                        mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                            "Failed to allocate map_buffer: size = %zu * %zu * %zu = %zu elements. "
                            "Not enough memory available. Consider processing fewer maps or reducing npix.",
                            N, nmaps, npix, N * nmaps * npix);
                    }
                    extract_map_data(map_buffer);
                    
                    // Create 3D views for batch processing (row-major)
                    array<size_t,3> map_shape = {N, nmaps, npix};
                    cmav<double,3> map_view(map_buffer.data(), map_shape);
                    
                    vector<complex<double>> alm_buffer;
                    try {
                        alm_buffer.resize(N * ncomp * nalm_dim);
                    } catch (const std::bad_alloc &e) {
                        mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                            "Failed to allocate alm_buffer: size = %zu * %zu * %zu = %zu elements. "
                            "Not enough memory available. Consider reducing lmax or batch size.",
                            N, ncomp, nalm_dim, N * ncomp * nalm_dim);
                    }
                    array<size_t,3> alm_shape = {N, ncomp, nalm_dim};
                    vmav<complex<double>,3> alm_view(alm_buffer.data(), alm_shape);
                    
                    // Call batch function (reuses setup internally)
                    adjoint_synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                           theta_view, nphi_view, phi0_view, ringstart_view,
                                           ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                    
                    // Copy from buffer to MATLAB (column-major)
                    for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                        for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                            for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                                size_t idx_buffer = ibatch * ncomp * nalm_dim + icomp * nalm_dim + ialm;
                                size_t idx_matlab = ibatch + icomp * N + ialm * N * ncomp;
                                alm_real[idx_matlab] = alm_buffer[idx_buffer].real();
                                alm_imag[idx_matlab] = alm_buffer[idx_buffer].imag();
                            }
                        }
                    }
                }
            } else {
                // Single mode: use regular function
                vector<double> map_buffer;
                try {
                    map_buffer.resize(nmaps * npix);
                } catch (const std::bad_alloc &e) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                        "Failed to allocate map_buffer: size = %zu * %zu = %zu elements. "
                        "Not enough memory available. Consider reducing npix.",
                        nmaps, npix, nmaps * npix);
                }
                vector<complex<double>> alm_buffer;
                try {
                    alm_buffer.resize(ncomp * nalm_dim);
                } catch (const std::bad_alloc &e) {
                    mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                        "Failed to allocate alm_buffer: size = %zu * %zu = %zu elements. "
                        "Not enough memory available. Consider reducing lmax (currently %zu).",
                        ncomp, nalm_dim, ncomp * nalm_dim, lmax);
                }
                
                // Extract data (handles both sparse and dense)
                extract_map_data(map_buffer);
                
                array<size_t,2> map_shape = {nmaps, npix};
                cmav<double,2> map_view(map_buffer.data(), map_shape);
                
                array<size_t,2> alm_shape = {ncomp, nalm_dim};
                vmav<complex<double>,2> alm_view(alm_buffer.data(), alm_shape);
                
                // Perform adjoint synthesis
                adjoint_synthesis(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                 theta_view, nphi_view, phi0_view, ringstart_view,
                                 ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                
                // Copy alm from buffer to MATLAB
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                        size_t idx_ducc = icomp * nalm_dim + ialm; // Row-major
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        alm_real[idx_matlab] = alm_buffer[idx_ducc].real();
                        alm_imag[idx_matlab] = alm_buffer[idx_ducc].imag();
                    }
                }
            }
            
        } else if (class_id == mxSINGLE_CLASS) {
            // For sparse batch mode, we'll create alm_arr after identifying non-zero maps
            // For other cases, alm_arr is already created above
            float *alm_real = nullptr;
            float *alm_imag = nullptr;
            if (alm_arr != nullptr) {
                alm_real = (float *)mxGetData(alm_arr);
                alm_imag = (float *)mxGetImagData(alm_arr);
            }
            
            // Helper function to extract sparse or dense data into dense buffer (float version)
            auto extract_map_data_float = [&](vector<float> &map_buffer) {
                if (is_sparse) {
                    // Handle sparse array: use CSC format (Compressed Sparse Column)
                    // Initialize buffer to zeros (sparse arrays have implicit zeros)
                    fill(map_buffer.begin(), map_buffer.end(), 0.0f);
                    
                    mwIndex *ir = mxGetIr(map_arr);  // Row indices
                    mwIndex *jc = mxGetJc(map_arr);  // Column pointers
                    const float *pr = (const float *)mxGetData(map_arr);  // Non-zero values
                    mwIndex nzmax = mxGetNzmax(map_arr);
                    
                    // For batch mode [N, nmaps, npix], sparse format is column-major
                    if (is_batch_mode) {
                        // For [N, nmaps, npix] in column-major: row index determines (ibatch, imap)
                        // column index determines ipix
                        for (mwIndex col = 0; col < npix; ++col) {
                            mwIndex row_start = jc[col];
                            mwIndex row_end = jc[col + 1];
                            for (mwIndex i = row_start; i < row_end; ++i) {
                                mwIndex row = ir[i];
                                float val = pr[i];
                                // Decompose row index: row = ibatch + imap * N
                                size_t ibatch = row % N;
                                size_t imap = row / N;
                                if (imap < nmaps) {
                                    // Convert to DUCC row-major: [N, nmaps, npix]
                                    size_t idx_ducc = ibatch * nmaps * npix + imap * npix + col;
                                    map_buffer[idx_ducc] = val;
                                }
                            }
                        }
                    } else {
                        // For [nmaps, npix]: row index is imap, column index is ipix
                        for (mwIndex col = 0; col < npix; ++col) {
                            mwIndex row_start = jc[col];
                            mwIndex row_end = jc[col + 1];
                            for (mwIndex i = row_start; i < row_end; ++i) {
                                mwIndex row = ir[i];
                                float val = pr[i];
                                if (row < nmaps) {
                                    // Convert to DUCC row-major: [nmaps, npix]
                                    size_t idx_ducc = row * npix + col;
                                    map_buffer[idx_ducc] = val;
                                }
                            }
                        }
                    }
                } else {
                    // Dense array: use existing conversion logic
                    const float *map_data = (const float *)mxGetData(map_arr);
                    if (is_batch_mode) {
                        // Convert from MATLAB column-major to DUCC row-major for all maps
                        for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                            for (size_t imap = 0; imap < nmaps; ++imap) {
                                for (size_t ipix = 0; ipix < npix; ++ipix) {
                                    // MATLAB column-major: [N, nmaps, npix]
                                    size_t idx_matlab = ibatch + imap * N + ipix * N * nmaps;
                                    // DUCC row-major: [N, nmaps, npix]
                                    size_t idx_ducc = ibatch * nmaps * npix + imap * npix + ipix;
                                    map_buffer[idx_ducc] = map_data[idx_matlab];
                                }
                            }
                        }
                    } else {
                        // Single map: [nmaps, npix]
                        for (size_t imap = 0; imap < nmaps; ++imap) {
                            for (size_t ipix = 0; ipix < npix; ++ipix) {
                                size_t idx_matlab = imap + ipix * nmaps; // Column-major
                                size_t idx_ducc = imap * npix + ipix; // Row-major
                                map_buffer[idx_ducc] = map_data[idx_matlab];
                            }
                        }
                    }
                }
            };
            
            if (is_batch_mode) {
                // For sparse arrays in batch mode, identify non-zero maps and process them together
                // to reuse computation (FFTs, basis functions computed once, reused for all maps)
                if (is_sparse) {
                    // Create output array first
                    mwSize alm_dims[3] = {N, ncomp, nalm_dim};
                    alm_arr = mxCreateNumericArray(3, alm_dims, class_id, mxCOMPLEX);
                    if (alm_arr == nullptr) {
                        mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                            "Failed to create output array: dimensions [%zu, %zu, %zu]. "
                            "Not enough memory available. Consider reducing lmax or batch size.",
                            N, ncomp, nalm_dim);
                    }
                    alm_real = (float *)mxGetData(alm_arr);
                    alm_imag = (float *)mxGetImagData(alm_arr);
                    
                    // Get sparse array structure
                    mwIndex *ir = mxGetIr(map_arr);  // Row indices
                    mwIndex *jc = mxGetJc(map_arr);  // Column pointers
                    const float *pr = (const float *)mxGetData(map_arr);  // Non-zero values
                    size_t npix_total = nmaps * npix;  // Total columns in sparse array
                    
                    // Find which rows (maps) have non-zero elements
                    vector<bool> row_has_data(N, false);
                    vector<size_t> non_zero_row_indices;
                    non_zero_row_indices.reserve(N);  // Reserve space, but likely much fewer
                    
                    for (mwIndex col = 0; col < npix_total; ++col) {
                        mwIndex row_start = jc[col];
                        mwIndex row_end = jc[col + 1];
                        for (mwIndex i = row_start; i < row_end; ++i) {
                            mwIndex row = ir[i];
                            if (row < (mwIndex)N && !row_has_data[row]) {
                                row_has_data[row] = true;
                                non_zero_row_indices.push_back(row);
                            }
                        }
                    }
                    
                    // Sort row indices for efficient processing
                    sort(non_zero_row_indices.begin(), non_zero_row_indices.end());
                    size_t N_nonzero = non_zero_row_indices.size();
                    
                    if (N_nonzero == 0) {
                        // All maps are zero - output array is already initialized to zeros
                    } else {
                        // Try to process all non-zero maps together first for maximum computation reuse
                        // Only fall back to chunking if memory allocation fails
                        vector<float> map_buffer;
                        bool use_chunking = false;
                        size_t chunk_size = 0;
                        
                        try {
                            // Try to allocate for all non-zero maps at once
                            map_buffer.resize(N_nonzero * nmaps * npix, 0.0f);
                        } catch (const std::bad_alloc &e) {
                            // If allocation fails, fall back to chunking
                            use_chunking = true;
                            // Start with a reasonable chunk size (try larger first for better parallelism)
                            chunk_size = 1024;  // Process up to 1024 maps at a time
                            if (chunk_size > N_nonzero) chunk_size = N_nonzero;
                        }
                        
                        if (!use_chunking) {
                            // Extract sparse data for all non-zero maps
                            for (mwIndex col = 0; col < npix_total; ++col) {
                                mwIndex row_start = jc[col];
                                mwIndex row_end = jc[col + 1];
                                for (mwIndex i = row_start; i < row_end; ++i) {
                                    mwIndex row = ir[i];
                                    float val = pr[i];
                                    
                                    // Find position in non-zero batch
                                    auto it = lower_bound(non_zero_row_indices.begin(), non_zero_row_indices.end(), row);
                                    if (it != non_zero_row_indices.end() && *it == row) {
                                        size_t batch_idx = distance(non_zero_row_indices.begin(), it);
                                        
                                        // Column j represents pixel in flattened format
                                        // Map to [nmaps, npix]: component = j/npix, pixel = j%npix
                                        size_t imap = col / npix;
                                        size_t ipix = col % npix;
                                        if (imap < nmaps && ipix < npix) {
                                            // Convert to batch buffer: [N_nonzero, nmaps, npix]
                                            size_t idx_buffer = batch_idx * nmaps * npix + imap * npix + ipix;
                                            map_buffer[idx_buffer] = val;
                                        }
                                    }
                                }
                            }
                            
                            // Process all non-zero maps together using batch function
                            // This maximizes computation reuse and multithreading
                            array<size_t,3> map_shape = {N_nonzero, nmaps, npix};
                            cmav<float,3> map_view(map_buffer.data(), map_shape);
                            
                            vector<complex<float>> alm_buffer;
                            try {
                                alm_buffer.resize(N_nonzero * ncomp * nalm_dim);
                            } catch (const std::bad_alloc &e) {
                                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                                    "Failed to allocate alm_buffer: size = %zu * %zu * %zu = %zu elements. "
                                    "Not enough memory available. Consider reducing lmax.",
                                    N_nonzero, ncomp, nalm_dim, N_nonzero * ncomp * nalm_dim);
                            }
                            array<size_t,3> alm_shape = {N_nonzero, ncomp, nalm_dim};
                            vmav<complex<float>,3> alm_view(alm_buffer.data(), alm_shape);
                            
                            // Call batch function for all non-zero maps (maximizes computation reuse and multithreading)
                            adjoint_synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                                   theta_view, nphi_view, phi0_view, ringstart_view,
                                                   ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                            
                            // Copy results to output array
                            for (size_t i = 0; i < N_nonzero; ++i) {
                                size_t orig_row = non_zero_row_indices[i];
                                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                                    for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                                        size_t idx_buffer = i * ncomp * nalm_dim + icomp * nalm_dim + ialm;
                                        size_t idx_matlab = orig_row + icomp * N + ialm * N * ncomp;
                                        alm_real[idx_matlab] = alm_buffer[idx_buffer].real();
                                        alm_imag[idx_matlab] = alm_buffer[idx_buffer].imag();
                                    }
                                }
                            }
                        } else {
                            // Fall back to chunking if memory is limited
                            // Process non-zero maps in chunks
                            for (size_t chunk_start = 0; chunk_start < N_nonzero; ) {
                                size_t chunk_end = (chunk_start + chunk_size < N_nonzero) ? chunk_start + chunk_size : N_nonzero;
                                size_t chunk_N = chunk_end - chunk_start;
                                
                                // Allocate buffer for this chunk of non-zero maps
                                try {
                                    map_buffer.resize(chunk_N * nmaps * npix, 0.0f);
                                } catch (const std::bad_alloc &e) {
                                    // If chunk allocation fails, try smaller chunk
                                    if (chunk_N > 1) {
                                        chunk_size = chunk_N / 2;
                                        continue;  // Retry this chunk with smaller size
                                    } else {
                                        mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                                            "Failed to allocate map_buffer: size = %zu * %zu * %zu = %zu elements. "
                                            "Not enough memory available. Consider processing fewer maps or reducing npix.",
                                            chunk_N, nmaps, npix, chunk_N * nmaps * npix);
                                    }
                                }
                            
                            // Extract sparse data only for this chunk of non-zero maps
                            for (mwIndex col = 0; col < npix_total; ++col) {
                                mwIndex row_start = jc[col];
                                mwIndex row_end = jc[col + 1];
                                for (mwIndex i = row_start; i < row_end; ++i) {
                                    mwIndex row = ir[i];
                                    float val = pr[i];
                                    
                                    // Find position in non-zero batch
                                    auto it = lower_bound(non_zero_row_indices.begin(), non_zero_row_indices.end(), row);
                                    if (it != non_zero_row_indices.end() && *it == row) {
                                        size_t batch_idx = distance(non_zero_row_indices.begin(), it);
                                        
                                        // Check if this map is in the current chunk
                                        if (batch_idx >= chunk_start && batch_idx < chunk_end) {
                                            size_t chunk_idx = batch_idx - chunk_start;
                                            
                                            // Column j represents pixel in flattened format
                                            // Map to [nmaps, npix]: component = j/npix, pixel = j%npix
                                            size_t imap = col / npix;
                                            size_t ipix = col % npix;
                                            if (imap < nmaps && ipix < npix) {
                                                // Convert to chunk buffer: [chunk_N, nmaps, npix]
                                                size_t idx_buffer = chunk_idx * nmaps * npix + imap * npix + ipix;
                                                map_buffer[idx_buffer] = val;
                                            }
                                        }
                                    }
                                }
                            }
                            
                            // Process this chunk using batch function (reuses computation within chunk)
                            array<size_t,3> map_shape = {chunk_N, nmaps, npix};
                            cmav<float,3> map_view(map_buffer.data(), map_shape);
                            
                            vector<complex<float>> alm_buffer;
                            try {
                                alm_buffer.resize(chunk_N * ncomp * nalm_dim);
                            } catch (const std::bad_alloc &e) {
                                mexErrMsgIdAndTxt("DUCC0:SHT:AdjointSynthesis:MemoryError", 
                                    "Failed to allocate alm_buffer: size = %zu * %zu * %zu = %zu elements. "
                                    "Not enough memory available. Consider reducing lmax.",
                                    chunk_N, ncomp, nalm_dim, chunk_N * ncomp * nalm_dim);
                            }
                            array<size_t,3> alm_shape = {chunk_N, ncomp, nalm_dim};
                            vmav<complex<float>,3> alm_view(alm_buffer.data(), alm_shape);
                            
                            // Call batch function for this chunk (reuses computation, enables multithreading)
                            adjoint_synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                                   theta_view, nphi_view, phi0_view, ringstart_view,
                                                   ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                            
                            // Copy results to output array, mapping chunk indices to original row indices
                            for (size_t i = 0; i < chunk_N; ++i) {
                                size_t orig_row = non_zero_row_indices[chunk_start + i];
                                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                                    for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                                        size_t idx_buffer = i * ncomp * nalm_dim + icomp * nalm_dim + ialm;
                                        size_t idx_matlab = orig_row + icomp * N + ialm * N * ncomp;
                                        alm_real[idx_matlab] = alm_buffer[idx_buffer].real();
                                        alm_imag[idx_matlab] = alm_buffer[idx_buffer].imag();
                                    }
                                }
                            }
                            
                            // Move to next chunk
                            chunk_start = chunk_end;
                        }
                        }
                    }
                } else {
                    // Dense batch mode: use batch C++ function
                    // Convert from MATLAB column-major to DUCC row-major for all maps
                    vector<float> map_buffer;
                    extract_map_data_float(map_buffer);
                    
                    // Create 3D views for batch processing (row-major)
                    array<size_t,3> map_shape = {N, nmaps, npix};
                    cmav<float,3> map_view(map_buffer.data(), map_shape);
                    
                    vector<complex<float>> alm_buffer(N * ncomp * nalm_dim);
                    array<size_t,3> alm_shape = {N, ncomp, nalm_dim};
                    vmav<complex<float>,3> alm_view(alm_buffer.data(), alm_shape);
                    
                    // Call batch function (reuses setup internally)
                    adjoint_synthesis_batch(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                           theta_view, nphi_view, phi0_view, ringstart_view,
                                           ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                    
                    // Copy from buffer to MATLAB (column-major)
                    for (size_t ibatch = 0; ibatch < N; ++ibatch) {
                        for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                            for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                                size_t idx_buffer = ibatch * ncomp * nalm_dim + icomp * nalm_dim + ialm;
                                size_t idx_matlab = ibatch + icomp * N + ialm * N * ncomp;
                                alm_real[idx_matlab] = alm_buffer[idx_buffer].real();
                                alm_imag[idx_matlab] = alm_buffer[idx_buffer].imag();
                            }
                        }
                    }
                }
            } else {
                // Single mode: use regular function
                vector<float> map_buffer(nmaps * npix);
                vector<complex<float>> alm_buffer(ncomp * nalm_dim);
                
                // Extract data (handles both sparse and dense)
                extract_map_data_float(map_buffer);
                
                array<size_t,2> map_shape = {nmaps, npix};
                cmav<float,2> map_view(map_buffer.data(), map_shape);
                
                array<size_t,2> alm_shape = {ncomp, nalm_dim};
                vmav<complex<float>,2> alm_view(alm_buffer.data(), alm_shape);
                
                // Perform adjoint synthesis
                adjoint_synthesis(alm_view, map_view, spin, lmax, mstart_view, lstride,
                                 theta_view, nphi_view, phi0_view, ringstart_view,
                                 ringfactor_view, pixstride, nthreads, mode, theta_interpol);
                
                // Copy alm from buffer to MATLAB
                for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                    for (size_t ialm = 0; ialm < nalm_dim; ++ialm) {
                        size_t idx_ducc = icomp * nalm_dim + ialm; // Row-major
                        size_t idx_matlab = icomp + ialm * ncomp; // Column-major
                        alm_real[idx_matlab] = alm_buffer[idx_ducc].real();
                        alm_imag[idx_matlab] = alm_buffer[idx_ducc].imag();
                    }
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

