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

    py_mod = ducc0.ducc0();
    py_result = py_mod.healpix.nside2npix(int32(nside));
    npix = double(py_result);
end

