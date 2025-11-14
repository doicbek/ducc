function weights = get_gridweights(geometry, ntheta)
%GET_GRIDWEIGHTS Returns quadrature weights for a given grid geometry
%   WEIGHTS = GET_GRIDWEIGHTS(GEOMETRY, NTHETA) returns quadrature weights.
%
%   Parameters
%   ----------
%   geometry : string
%       Grid geometry: 'CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2'
%   ntheta : int
%       Number of rings in the grid
%
%   Returns
%   -------
%   weights : double array
%       Quadrature weights for individual rings
%
%   Note: These weights need to be divided by the number of pixels per ring
%   to obtain actual quadrature weights for a particular map.

    if nargin < 2
        error('DUCC0:InvalidInput', 'Both geometry and ntheta are required');
    end
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.sht.get_gridweights(char(geometry), int32(ntheta));
    weights = ducc0.util.numpy2matlab(py_result);
end

