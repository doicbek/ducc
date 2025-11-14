/*
 * DUCC0 SHT Get Gridweights MEX Interface
 * 
 * MATLAB MEX gateway for getting quadrature weights for a given grid geometry
 * 
 * Usage:
 *   weights = ducc0_sht_get_gridweights_mex(geometry, ntheta)
 * 
 * Inputs:
 *   geometry: Grid geometry string ('CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2')
 *   ntheta: Number of rings in the grid
 * 
 * Output:
 *   weights: Quadrature weights (double array, size [ntheta])
 */

#include "mex.h"
#include "ducc0_mex_utils.h"
#include "ducc0/sht/sht.h"
#include "ducc0/infra/error_handling.h"
#include <vector>
#include <string>

using namespace ducc0;
using namespace ducc0_mex;
using namespace std;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    try {
        // Check inputs
        if (nrhs < 2) {
            mexErrMsgIdAndTxt("DUCC0:SHT:GetGridweights:InputError", 
                "At least 2 inputs required: geometry, ntheta");
        }
        
        // Parse parameters
        string geometry = getStringParam(prhs[0]);
        size_t ntheta = (size_t)mxGetScalar(prhs[1]);
        
        if (ntheta == 0) {
            mexErrMsgIdAndTxt("DUCC0:SHT:GetGridweights:InputError", 
                "ntheta must be greater than 0");
        }
        
        // Get weights from DUCC
        vector<double> weights_buffer(ntheta);
        vmav<double,1> weights_view(weights_buffer.data(), {ntheta}, vector<ptrdiff_t>());
        
        get_gridweights(geometry, weights_view);
        
        // Create output array and copy data
        mxArray *weights_arr = mxCreateDoubleMatrix(ntheta, 1, mxREAL);
        double *weights_data = mxGetPr(weights_arr);
        
        for (size_t i = 0; i < ntheta; ++i) {
            weights_data[i] = weights_buffer[i];
        }
        
        plhs[0] = weights_arr;
        
    } catch (const exception &e) {
        handleDuccError(e);
    }
}

