function npix = nside2npix(nside)
%NSIDE2NPIX Convert HEALPix nside to number of pixels
%   NPIX = NSIDE2NPIX(NSIDE) returns the number of pixels for a given nside.
%
%   Parameters
%   ----------
%   nside : int
%       HEALPix nside parameter (must be power of 2)
%
%   Returns
%   -------
%   npix : int
%       Number of pixels = 12 * nside^2

    % Simple MATLAB implementation (no MEX needed for this simple function)
    if nargin < 1
        error('DUCC0:InvalidInput', 'nside is required');
    end
    
    if ~isnumeric(nside) || ~isscalar(nside) || nside < 1
        error('DUCC0:InvalidInput', 'nside must be a positive integer');
    end
    
    npix = 12 * double(nside)^2;
end
