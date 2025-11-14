function [theta, phi] = pix2ang(nside, pix, varargin)
%PIX2ANG Convert HEALPix pixel index to angular coordinates
%   [THETA, PHI] = PIX2ANG(NSIDE, PIX) converts pixel index to (theta, phi).
%   [THETA, PHI] = PIX2ANG(NSIDE, PIX, 'nest', true) uses NESTED ordering.
%   [THETA, PHI] = PIX2ANG(NSIDE, PIX, 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   nside : int
%       HEALPix nside parameter
%   pix : int or int array
%       Pixel index(ices)
%   nest : bool, optional
%       Use NESTED ordering instead of RING (default: false)
%   nthreads : int, optional
%       Number of threads to use (default: 0 = auto)
%
%   Returns
%   -------
%   theta : double or double array
%       Colatitude in radians (0 to pi)
%   phi : double or double array
%       Azimuth in radians (0 to 2*pi)

    p = inputParser;
    addRequired(p, 'nside', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'pix', @(x) isnumeric(x));
    addParameter(p, 'nest', false, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, nside, pix, varargin{:});
    
    [theta, phi] = ducc0_healpix_pix2ang_mex(double(p.Results.nside), double(p.Results.pix), ...
        logical(p.Results.nest), double(p.Results.nthreads));
end
