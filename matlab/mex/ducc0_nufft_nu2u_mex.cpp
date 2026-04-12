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
        
        // Convert coord from MATLAB column-major to DUCC row-major
        vector<double> coord_buffer(npoints * ndim);
        const double *coord_data = mxGetPr(coord_arr);
        for (size_t ipoint = 0; ipoint < npoints; ++ipoint) {
            for (size_t idim = 0; idim < ndim; ++idim) {
                coord_buffer[ipoint * ndim + idim] = coord_data[ipoint + idim * npoints];
            }
        }
        array<size_t,2> coord_shape = {npoints, ndim};
        cmav<double,2> coord_view(coord_buffer.data(), coord_shape);

        // Compute grid shape vector and total element count
        size_t grid_nelem = 1;
        vector<size_t> grid_shape_vec(ndim);
        for (size_t i = 0; i < ndim; ++i) {
            grid_shape_vec[i] = gridshape[i];
            grid_nelem *= gridshape[i];
        }

        if (ncomp == 1) {
            // Single-component: copy points and call nu2u
            const double *p_real = mxGetPr(points_arr);
            const double *p_imag = mxGetPi(points_arr);
            vector<complex<double>> points_buffer(npoints);
            for (size_t i = 0; i < npoints; ++i) {
                points_buffer[i] = complex<double>(p_real[i], p_imag ? p_imag[i] : 0.0);
            }
            array<size_t,1> pts_shape = {npoints};
            cmav<complex<double>,1> points_view(points_buffer.data(), pts_shape);

            vector<complex<double>> grid_buffer(grid_nelem, {0.0, 0.0});
            vfmav<complex<double>> grid_view(grid_buffer.data(), grid_shape_vec);

            nu2u<double, double, double, double, double>(
                coord_view, points_view, forward, epsilon, nthreads,
                grid_view, verbosity, sigma_min, sigma_max, periodicity, fft_order);

            copyBufferToMatlab<complex<double>>(grid_buffer.data(), grid_arr, grid_shape_vec);

        } else {
            // Multi-component: reuse Nufft object across components
            Nufft<double, double, double> nufft(true, npoints, grid_shape_vec,
                epsilon, nthreads, sigma_min, sigma_max, periodicity, fft_order);

            const double *p_real = mxGetPr(points_arr);
            const double *p_imag = mxGetPi(points_arr);
            double *g_real = mxGetPr(grid_arr);
            double *g_imag = mxGetPi(grid_arr);

            // Temporary single-component MATLAB array for column-major conversion
            mwSize temp_dims[3];
            for (size_t i = 0; i < ndim; ++i) temp_dims[i] = gridshape[ndim - 1 - i];
            mxArray *temp_arr = mxCreateNumericArray(ndim, temp_dims, mxDOUBLE_CLASS, mxCOMPLEX);

            for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                // points layout in MATLAB: [ncomp, npoints] column-major
                vector<complex<double>> points_buffer(npoints);
                for (size_t i = 0; i < npoints; ++i) {
                    size_t idx = icomp + i * ncomp;
                    points_buffer[i] = complex<double>(p_real[idx], p_imag ? p_imag[idx] : 0.0);
                }
                array<size_t,1> pts_shape = {npoints};
                cmav<complex<double>,1> points_view(points_buffer.data(), pts_shape);

                vector<complex<double>> grid_buffer(grid_nelem, {0.0, 0.0});
                vfmav<complex<double>> grid_view(grid_buffer.data(), grid_shape_vec);
                nufft.nu2u(forward, verbosity, coord_view, points_view, grid_view);

                // Convert row-major grid_buffer → column-major temp_arr
                copyBufferToMatlab<complex<double>>(grid_buffer.data(), temp_arr, grid_shape_vec);

                // Interleave this component into the output array
                // grid_arr layout: [ncomp, g_{ndim-1}, ..., g_0] column-major
                // element [icomp, flat_j] → linear index icomp + flat_j * ncomp
                const double *t_real = mxGetPr(temp_arr);
                const double *t_imag = mxGetPi(temp_arr);
                for (size_t i = 0; i < grid_nelem; ++i) {
                    g_real[icomp + i * ncomp] = t_real[i];
                    g_imag[icomp + i * ncomp] = t_imag[i];
                }
            }
            mxDestroyArray(temp_arr);
        }

        plhs[0] = grid_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}



