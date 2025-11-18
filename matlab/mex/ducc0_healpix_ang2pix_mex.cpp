/*
 * DUCC0 HEALPix ANG2PIX MEX Interface
 * 
 * MATLAB MEX gateway for converting angular coordinates to HEALPix pixel indices
 * 
 * Usage:
 *   pix = ducc0_healpix_ang2pix_mex(nside, theta, phi, nest, nthreads)
 * 
 * Inputs:
 *   nside: HEALPix nside parameter (scalar)
 *   theta: Colatitude in radians (scalar or array, 0 to pi)
 *   phi: Azimuth in radians (scalar or array, 0 to 2*pi)
 *   nest: Use NESTED ordering instead of RING (optional, default: false)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   pix: Pixel index (same shape as theta/phi)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/healpix/healpix_base.h"
#include "ducc0/math/pointing.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ducc0;
using namespace std;

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
        if (nrhs < 3) {
            mexErrMsgIdAndTxt("DUCC0:HEALPix:ANG2PIX:InputError", 
                "At least 3 inputs required: nside, theta, phi");
        }
        
        // Parse parameters
        int64_t nside = (int64_t)mxGetScalar(prhs[0]);
        const mxArray *theta_arr = prhs[1];
        const mxArray *phi_arr = prhs[2];
        bool nest = getOptionalParam<bool>(nrhs > 3 ? prhs[3] : nullptr, false);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 4 ? prhs[4] : nullptr, 0);
        
        if (nside <= 0) {
            mexErrMsgIdAndTxt("DUCC0:HEALPix:ANG2PIX:InputError", 
                "nside must be positive");
        }
        
        // Check theta and phi dimensions
        mwSize theta_ndim = mxGetNumberOfDimensions(theta_arr);
        mwSize phi_ndim = mxGetNumberOfDimensions(phi_arr);
        const mwSize *theta_dims = mxGetDimensions(theta_arr);
        const mwSize *phi_dims = mxGetDimensions(phi_arr);
        
        size_t nelem = mxGetNumberOfElements(theta_arr);
        if (mxGetNumberOfElements(phi_arr) != nelem) {
            mexErrMsgIdAndTxt("DUCC0:HEALPix:ANG2PIX:InputError", 
                "theta and phi must have the same number of elements");
        }
        
        // Create HEALPix base
        detail_healpix::Ordering_Scheme scheme = nest ? detail_healpix::NEST : detail_healpix::RING;
        Healpix_Base2 base(nside, scheme, SET_NSIDE);
        
        // Create output array
        mxArray *pix_arr = mxCreateNumericArray(theta_ndim, theta_dims, mxINT64_CLASS, mxREAL);
        int64_t *pix_data = (int64_t *)mxGetData(pix_arr);
        
        // Get input data
        const double *theta_data = mxGetPr(theta_arr);
        const double *phi_data = mxGetPr(phi_arr);
        
        // Convert to pixel indices
        for (size_t i = 0; i < nelem; ++i) {
            double theta = theta_data[i];
            double phi = phi_data[i];
            
            if (theta < 0 || theta > M_PI) {
                mexErrMsgIdAndTxt("DUCC0:HEALPix:ANG2PIX:InputError", 
                    "theta must be in range [0, pi]");
            }
            
            pointing ptg(theta, phi);
            pix_data[i] = base.ang2pix(ptg);
        }
        
        plhs[0] = pix_arr;
        
    } catch (const exception &e) {
        ducc0_mex::handleDuccError(e);
    }
}

