function nside = npix2nside(npix)
%NPIX2NSIDE Convert number of pixels to HEALPix nside
%   NSIDE = NPIX2NSIDE(NPIX) returns the nside for a given number of pixels.
%
%   Parameters
%   ----------
%   npix : int
%       Number of pixels (must be 12 * nside^2 for some nside)
%
%   Returns
%   -------
%   nside : int
%       HEALPix nside parameter

    % Simple MATLAB implementation (no MEX needed for this simple function)
    if nargin < 1
        error('DUCC0:InvalidInput', 'npix is required');
    end
    
    if ~isnumeric(npix) || ~isscalar(npix) || npix < 12
        error('DUCC0:InvalidInput', 'npix must be at least 12');
    end
    
    nside_sq = double(npix) / 12;
    nside = sqrt(nside_sq);
    
    % Check if it's a valid nside (should be integer, typically power of 2)
    if abs(nside - round(nside)) > 1e-10
        error('DUCC0:InvalidInput', 'npix is not a valid HEALPix pixel count');
    end
    
    nside = round(nside);
end
