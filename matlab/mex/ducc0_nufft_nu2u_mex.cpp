/*
 * DUCC0 NUFFT NU2U MEX Interface
 * 
 * MATLAB MEX gateway for non-uniform to uniform FFT
 * 
 * Usage:
 *   grid = ducc0_nufft_nu2u_mex(points, coord, forward, epsilon, nthreads, gridshape, verbosity, sigma_min, sigma_max, periodicity, fft_order)
 * 
 * Inputs:
 *   points: Non-uniform points (complex array, shape [npoints] or [ncomp, npoints])
 *   coord: Coordinates (real array, shape [npoints, ndim])
 *   forward: Forward transform flag (optional, default: true)
 *   epsilon: Accuracy parameter (optional, default: 1e-10)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 *   gridshape: Grid shape (optional, will be computed if not provided)
 *   verbosity: Verbosity level (optional, default: 0)
 *   sigma_min: Minimum oversampling factor (optional, default: 1.2)
 *   sigma_max: Maximum oversampling factor (optional, default: 2.5)
 *   periodicity: Periodicity (optional, default: 2*pi for each dimension)
 *   fft_order: FFT order flag (optional, default: true)
 * 
 * Output:
 *   grid: Uniform grid (complex array, shape [gridshape] or [ncomp, gridshape])
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/nufft/nufft.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>
#include <cmath>

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to get optional parameter
template<typename T>
T getOptionalParam(const mxArray *arr, T default_value)
{
    if (arr == nullptr || mxIsEmpty(arr)) {
        return default_value;
    }
    if constexpr (is_same_v<T, bool>) {
        return mxGetScalar(arr) != 0;
    } else {
        return static_cast<T>(mxGetScalar(arr));
    }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 2) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "At least 2 inputs required: points, coord");
        }
        
        const mxArray *points_arr = prhs[0];
        const mxArray *coord_arr = prhs[1];
        
        if (mxIsEmpty(points_arr) || mxIsEmpty(coord_arr)) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "points and coord cannot be empty");
        }
        
        if (!mxIsComplex(points_arr)) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "points must be complex");
        }
        
        if (mxIsComplex(coord_arr)) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "coord must be real");
        }
        
        // Parse optional parameters
        bool forward = getOptionalParam<bool>(nrhs > 2 ? prhs[2] : nullptr, true);
        double epsilon = getOptionalParam<double>(nrhs > 3 ? prhs[3] : nullptr, 1e-10);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 4 ? prhs[4] : nullptr, 0);
        const mxArray *gridshape_arr = (nrhs > 5) ? prhs[5] : nullptr;
        size_t verbosity = getOptionalParam<size_t>(nrhs > 6 ? prhs[6] : nullptr, 0);
        double sigma_min = getOptionalParam<double>(nrhs > 7 ? prhs[7] : nullptr, 1.2);
        double sigma_max = getOptionalParam<double>(nrhs > 8 ? prhs[8] : nullptr, 2.5);
        const mxArray *periodicity_arr = (nrhs > 9) ? prhs[9] : nullptr;
        bool fft_order = getOptionalParam<bool>(nrhs > 10 ? prhs[10] : nullptr, true);
        
        // Get points dimensions
        mwSize points_ndim = mxGetNumberOfDimensions(points_arr);
        const mwSize *points_dims = mxGetDimensions(points_arr);
        
        size_t ncomp = 1;
        size_t npoints = 0;
        if (points_ndim == 1) {
            npoints = points_dims[0];
        } else if (points_ndim == 2) {
            ncomp = points_dims[0];
            npoints = points_dims[1];
        } else {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "points must be 1D or 2D array");
        }
        
        // Get coord dimensions
        mwSize coord_ndim = mxGetNumberOfDimensions(coord_arr);
        const mwSize *coord_dims = mxGetDimensions(coord_arr);
        
        if (coord_ndim != 2) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "coord must be 2D array [npoints, ndim]");
        }
        
        size_t npoints_coord = coord_dims[0];
        size_t ndim = coord_dims[1];
        
        if (npoints != npoints_coord) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "points and coord must have matching npoints");
        }
        
        if (ndim < 1 || ndim > 3) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                "ndim must be 1, 2, or 3");
        }
        
        // Get periodicity (default: 2*pi for each dimension)
        vector<double> periodicity(ndim, 2.0 * M_PI);
        if (periodicity_arr != nullptr && !mxIsEmpty(periodicity_arr)) {
            size_t nperiod = mxGetNumberOfElements(periodicity_arr);
            if (nperiod == 1) {
                double val = mxGetScalar(periodicity_arr);
                for (size_t i = 0; i < ndim; ++i) {
                    periodicity[i] = val;
                }
            } else if (nperiod == ndim) {
                double *periodicity_data = mxGetPr(periodicity_arr);
                for (size_t i = 0; i < ndim; ++i) {
                    periodicity[i] = periodicity_data[i];
                }
            } else {
                mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                    "periodicity must be scalar or array of size ndim");
            }
        }
        
        // Get grid shape (default: computed from coord)
        vector<size_t> gridshape(ndim);
        if (gridshape_arr != nullptr && !mxIsEmpty(gridshape_arr)) {
            size_t ngrid = mxGetNumberOfElements(gridshape_arr);
            if (ngrid != ndim) {
                mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:InputError", 
                    "gridshape must have size ndim");
            }
            double *gridshape_data = mxGetPr(gridshape_arr);
            for (size_t i = 0; i < ndim; ++i) {
                gridshape[i] = (size_t)gridshape_data[i];
            }
        } else {
            // Compute grid shape from coord (simplified: use good_size)
            // For now, use a simple default
            for (size_t i = 0; i < ndim; ++i) {
                gridshape[i] = 64;  // Default, should be computed properly
            }
        }
        
        // Determine data type
        mxClassID points_class_id = mxGetClassID(points_arr);
        mxClassID coord_class_id = mxGetClassID(coord_arr);
        
        // For now, only support double precision
        if (points_class_id != mxDOUBLE_CLASS || coord_class_id != mxDOUBLE_CLASS) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:TypeError", 
                "Only double precision is currently supported");
        }
        
        // Create output array
        mwSize grid_ndim = ndim;
        if (ncomp > 1) {
            grid_ndim = ndim + 1;
        }
        mwSize *grid_dims = new mwSize[grid_ndim];
        if (ncomp > 1) {
            grid_dims[0] = ncomp;
            for (size_t i = 0; i < ndim; ++i) {
                grid_dims[i + 1] = gridshape[ndim - 1 - i]; // Convert to column-major
            }
        } else {
            for (size_t i = 0; i < ndim; ++i) {
                grid_dims[i] = gridshape[ndim - 1 - i]; // Convert to column-major
            }
        }
        
        mxArray *grid_arr = mxCreateNumericArray(grid_ndim, grid_dims, mxDOUBLE_CLASS, mxCOMPLEX);
        delete[] grid_dims;
        
        // Process: convert MATLAB arrays to DUCC format and call nu2u
        // This is a simplified implementation - full implementation would handle
        // array conversion properly
        
        // For now, return a placeholder
        // Full implementation requires proper array conversion and NUFFT setup
        mexErrMsgIdAndTxt("DUCC0:NUFFT:NU2U:NotImplemented", 
            "NUFFT nu2u MEX function is not yet fully implemented. Complex array conversion required.");
        
        plhs[0] = grid_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

