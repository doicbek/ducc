/*
 * DUCC0 NUFFT U2NU MEX Interface
 * 
 * MATLAB MEX gateway for uniform to non-uniform FFT
 * 
 * Usage:
 *   points = ducc0_nufft_u2nu_mex(grid, coord, forward, epsilon, periodicity, fft_order, nthreads)
 * 
 * Inputs:
 *   grid: Uniform grid (complex array, shape [gridshape] or [ncomp, gridshape])
 *   coord: Coordinates (real array, shape [npoints, ndim])
 *   forward: Forward transform flag (optional, default: true)
 *   epsilon: Accuracy parameter (optional, default: 1e-12)
 *   periodicity: Periodicity (optional, default: 2*pi for each dimension)
 *   fft_order: FFT order flag (optional, default: false)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   points: Non-uniform points (complex array, shape [npoints] or [ncomp, npoints])
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/nufft/nufft.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <complex>
#include <cmath>
#include <array>

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
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "At least 2 inputs required: grid, coord");
        }
        
        const mxArray *grid_arr = prhs[0];
        const mxArray *coord_arr = prhs[1];
        
        if (mxIsEmpty(grid_arr) || mxIsEmpty(coord_arr)) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "grid and coord cannot be empty");
        }
        
        if (!mxIsComplex(grid_arr)) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "grid must be complex");
        }
        
        if (mxIsComplex(coord_arr)) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "coord must be real");
        }
        
        // Parse optional parameters
        bool forward = getOptionalParam<bool>(nrhs > 2 ? prhs[2] : nullptr, true);
        double epsilon = getOptionalParam<double>(nrhs > 3 ? prhs[3] : nullptr, 1e-12);
        const mxArray *periodicity_arr = (nrhs > 4) ? prhs[4] : nullptr;
        bool fft_order = getOptionalParam<bool>(nrhs > 5 ? prhs[5] : nullptr, false);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 6 ? prhs[6] : nullptr, 0);
        
        // Get grid dimensions
        mwSize grid_ndim = mxGetNumberOfDimensions(grid_arr);
        const mwSize *grid_dims = mxGetDimensions(grid_arr);
        
        // Get coord dimensions
        mwSize coord_ndim = mxGetNumberOfDimensions(coord_arr);
        const mwSize *coord_dims = mxGetDimensions(coord_arr);
        
        if (coord_ndim != 2) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "coord must be 2D array [npoints, ndim]");
        }
        
        size_t npoints = coord_dims[0];
        size_t ndim = coord_dims[1];
        
        if (ndim < 1 || ndim > 3) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "ndim must be 1, 2, or 3");
        }
        
        // Determine if grid has leading component dimension
        size_t ncomp = 1;
        size_t grid_data_ndim = grid_ndim;
        if (grid_ndim == ndim + 1) {
            ncomp = grid_dims[0];
            grid_data_ndim = ndim;
        } else if (grid_ndim != ndim) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                "grid dimensions do not match ndim");
        }
        
        // Get grid shape (reverse order for row-major)
        vector<size_t> gridshape(ndim);
        if (ncomp > 1) {
            for (size_t i = 0; i < ndim; ++i) {
                gridshape[ndim - 1 - i] = grid_dims[i + 1]; // Convert from column-major
            }
        } else {
            for (size_t i = 0; i < ndim; ++i) {
                gridshape[ndim - 1 - i] = grid_dims[i]; // Convert from column-major
            }
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
                mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:InputError", 
                    "periodicity must be scalar or array of size ndim");
            }
        }
        
        // Determine data type
        mxClassID grid_class_id = mxGetClassID(grid_arr);
        mxClassID coord_class_id = mxGetClassID(coord_arr);
        
        // For now, only support double precision
        if (grid_class_id != mxDOUBLE_CLASS || coord_class_id != mxDOUBLE_CLASS) {
            mexErrMsgIdAndTxt("DUCC0:NUFFT:U2NU:TypeError", 
                "Only double precision is currently supported");
        }
        
        // Create output array
        mwSize points_ndim = (ncomp > 1) ? 2 : 1;
        mwSize *points_dims = new mwSize[points_ndim];
        if (ncomp > 1) {
            points_dims[0] = ncomp;
            points_dims[1] = npoints;
        } else {
            points_dims[0] = npoints;
        }
        
        mxArray *points_arr = mxCreateNumericArray(points_ndim, points_dims, mxDOUBLE_CLASS, mxCOMPLEX);
        delete[] points_dims;
        
        // Process based on number of components
        if (ncomp == 1) {
            // Single component case
            // Convert coord from MATLAB to DUCC format
            vector<double> coord_buffer(npoints * ndim);
            const double *coord_data = mxGetPr(coord_arr);
            for (size_t ipoint = 0; ipoint < npoints; ++ipoint) {
                for (size_t idim = 0; idim < ndim; ++idim) {
                    size_t idx_matlab = ipoint + idim * npoints; // Column-major
                    size_t idx_ducc = ipoint * ndim + idim; // Row-major
                    coord_buffer[idx_ducc] = coord_data[idx_matlab];
                }
            }
            
            cmav<double,2> coord_view(coord_buffer.data(), {npoints, ndim}, vector<ptrdiff_t>());
            
            // Convert grid from MATLAB to DUCC format
            size_t grid_nelem = 1;
            for (size_t i = 0; i < ndim; ++i) {
                grid_nelem *= gridshape[i];
            }
            vector<complex<double>> grid_buffer(grid_nelem);
            
            // Use utility function to copy from MATLAB to buffer
            copyMatlabToBuffer<complex<double>>(grid_arr, grid_buffer.data(), gridshape);
            
            vector<size_t> grid_shape_vec(ndim);
            for (size_t i = 0; i < ndim; ++i) {
                grid_shape_vec[i] = gridshape[i];
            }
            cfmav<complex<double>> grid_view(grid_buffer.data(), grid_shape_vec, vector<ptrdiff_t>());
            
            // Create points buffer
            vector<complex<double>> points_buffer(npoints);
            array<size_t,1> points_shape = {npoints};
            vmav<complex<double>,1> points_view(points_buffer.data(), points_shape);
            
            // Call u2nu
            u2nu<double, double, double, double, double>(
                coord_view, grid_view, forward, epsilon, nthreads,
                points_view, 0, 1.2, 2.5, periodicity, fft_order);
            
            // Copy points from buffer to MATLAB
            double *points_real = mxGetPr(points_arr);
            double *points_imag = mxGetPi(points_arr);
            for (size_t i = 0; i < npoints; ++i) {
                points_real[i] = points_buffer[i].real();
                points_imag[i] = points_buffer[i].imag();
            }
            
        } else {
            // Multi-component case - process each component
            // Convert coord from MATLAB to DUCC format
            vector<double> coord_buffer(npoints * ndim);
            const double *coord_data = mxGetPr(coord_arr);
            for (size_t ipoint = 0; ipoint < npoints; ++ipoint) {
                for (size_t idim = 0; idim < ndim; ++idim) {
                    size_t idx_matlab = ipoint + idim * npoints; // Column-major
                    size_t idx_ducc = ipoint * ndim + idim; // Row-major
                    coord_buffer[idx_ducc] = coord_data[idx_matlab];
                }
            }
            array<size_t,2> coord_shape = {npoints, ndim};
            cmav<double,2> coord_view(coord_buffer.data(), coord_shape);
            
            // Create Nufft object for reuse
            vector<size_t> grid_shape_vec(ndim);
            for (size_t i = 0; i < ndim; ++i) {
                grid_shape_vec[i] = gridshape[i];
            }
            Nufft<double, double, double> nufft(false, npoints, grid_shape_vec,
                epsilon, nthreads, 1.2, 2.5, periodicity, fft_order);
            
            // Calculate grid_nelem
            size_t grid_nelem = 1;
            for (size_t i = 0; i < ndim; ++i) {
                grid_nelem *= gridshape[i];
            }
            
            // Process each component
            double *points_real = mxGetPr(points_arr);
            double *points_imag = mxGetPi(points_arr);
            const double *grid_real = mxGetPr(grid_arr);
            const double *grid_imag = mxGetPi(grid_arr);
            
            for (size_t icomp = 0; icomp < ncomp; ++icomp) {
                // Extract grid for this component
                vector<complex<double>> grid_buffer(grid_nelem);
                size_t grid_offset = icomp * grid_nelem;
                for (size_t i = 0; i < grid_nelem; ++i) {
                    size_t idx_matlab = grid_offset + i;
                    grid_buffer[i] = complex<double>(grid_real[idx_matlab], grid_imag[idx_matlab]);
                }
                
                // Convert to DUCC format (simplified - assumes column-major input)
                // TODO: Use proper conversion utility
                cfmav<complex<double>> grid_view(grid_buffer.data(), grid_shape_vec, vector<ptrdiff_t>());
                
                // Create points buffer for this component
                vector<complex<double>> points_buffer(npoints);
                array<size_t,1> points_shape = {npoints};
                vmav<complex<double>,1> points_view(points_buffer.data(), points_shape);
                
                // Call u2nu for this component
                nufft.u2nu(forward, 0, grid_view, coord_view, points_view);
                
                // Copy points to MATLAB output
                for (size_t i = 0; i < npoints; ++i) {
                    size_t idx_matlab = icomp + i * ncomp; // Column-major
                    points_real[idx_matlab] = points_buffer[i].real();
                    points_imag[idx_matlab] = points_buffer[i].imag();
                }
            }
        }
        
        plhs[0] = points_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

