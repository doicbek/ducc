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

    py_mod = ducc0.ducc0();
    py_result = py_mod.healpix.npix2nside(int32(npix));
    nside = double(py_result);
end

