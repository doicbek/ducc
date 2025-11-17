/*
 * DUCC0 HEALPix PIX2ANG MEX Interface
 * 
 * MATLAB MEX gateway for converting HEALPix pixel indices to angular coordinates
 * 
 * Usage:
 *   [theta, phi] = ducc0_healpix_pix2ang_mex(nside, pix, nest, nthreads)
 * 
 * Inputs:
 *   nside: HEALPix nside parameter (scalar)
 *   pix: Pixel indices (scalar or array)
 *   nest: Use NESTED ordering instead of RING (optional, default: false)
 *   nthreads: Number of threads (optional, default: 0 = auto)
 * 
 * Output:
 *   theta: Colatitude in radians (same shape as pix)
 *   phi: Azimuth in radians (same shape as pix)
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/healpix/healpix_base.h"
#include "ducc0/math/pointing.h"
#include "ducc0/infra/error_handling.h"
#include <vector>

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
        if (nrhs < 2) {
            mexErrMsgIdAndTxt("DUCC0:HEALPix:PIX2ANG:InputError", 
                "At least 2 inputs required: nside, pix");
        }
        
        // Parse parameters
        int64_t nside = (int64_t)mxGetScalar(prhs[0]);
        const mxArray *pix_arr = prhs[1];
        bool nest = getOptionalParam<bool>(nrhs > 2 ? prhs[2] : nullptr, false);
        size_t nthreads = getOptionalParam<size_t>(nrhs > 3 ? prhs[3] : nullptr, 0);
        
        if (nside <= 0) {
            mexErrMsgIdAndTxt("DUCC0:HEALPix:PIX2ANG:InputError", 
                "nside must be positive");
        }
        
        // Get pix dimensions
        mwSize pix_ndim = mxGetNumberOfDimensions(pix_arr);
        const mwSize *pix_dims = mxGetDimensions(pix_arr);
        size_t nelem = mxGetNumberOfElements(pix_arr);
        
        // Create HEALPix base
        Ordering_Scheme scheme = nest ? NEST : RING;
        Healpix_Base2 base(nside, scheme, SET_NSIDE);
        
        // Create output arrays
        mxArray *theta_arr = mxCreateNumericArray(pix_ndim, pix_dims, mxDOUBLE_CLASS, mxREAL);
        mxArray *phi_arr = mxCreateNumericArray(pix_ndim, pix_dims, mxDOUBLE_CLASS, mxREAL);
        
        double *theta_data = mxGetPr(theta_arr);
        double *phi_data = mxGetPr(phi_arr);
        
        // Get input data
        mxClassID pix_class = mxGetClassID(pix_arr);
        if (pix_class == mxINT64_CLASS) {
            const int64_t *pix_data = (const int64_t *)mxGetData(pix_arr);
            for (size_t i = 0; i < nelem; ++i) {
                pointing ptg = base.pix2ang(pix_data[i]);
                theta_data[i] = ptg.theta;
                phi_data[i] = ptg.phi;
            }
        } else if (pix_class == mxINT32_CLASS) {
            const int32_t *pix_data = (const int32_t *)mxGetData(pix_arr);
            for (size_t i = 0; i < nelem; ++i) {
                pointing ptg = base.pix2ang((int64_t)pix_data[i]);
                theta_data[i] = ptg.theta;
                phi_data[i] = ptg.phi;
            }
        } else if (pix_class == mxDOUBLE_CLASS) {
            const double *pix_data = mxGetPr(pix_arr);
            for (size_t i = 0; i < nelem; ++i) {
                pointing ptg = base.pix2ang((int64_t)pix_data[i]);
                theta_data[i] = ptg.theta;
                phi_data[i] = ptg.phi;
            }
        } else {
            mexErrMsgIdAndTxt("DUCC0:HEALPix:PIX2ANG:TypeError", 
                "pix must be int32, int64, or double");
        }
        
        plhs[0] = theta_arr;
        if (nlhs > 1) {
            plhs[1] = phi_arr;
        }
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}



