function pix = ang2pix(nside, theta, phi, varargin)
%ANG2PIX Convert angular coordinates to HEALPix pixel index
%   PIX = ANG2PIX(NSIDE, THETA, PHI) converts (theta, phi) to pixel index.
%   PIX = ANG2PIX(NSIDE, THETA, PHI, 'nest', true) uses NESTED ordering.
%   PIX = ANG2PIX(NSIDE, THETA, PHI, 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   nside : int
%       HEALPix nside parameter
%   theta : double or array
%       Colatitude in radians (0 to pi)
%   phi : double or array
%       Azimuth in radians (0 to 2*pi)
%   nest : bool, optional
%       Use NESTED ordering instead of RING (default: false)
%   nthreads : int, optional
%       Number of threads to use (default: 0 = auto)
%
%   Returns
%   -------
%   pix : int or int array
%       Pixel index(ices)

    p = inputParser;
    addRequired(p, 'nside', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'theta', @(x) isnumeric(x));
    addRequired(p, 'phi', @(x) isnumeric(x));
    addParameter(p, 'nest', false, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, nside, theta, phi, varargin{:});
    
    pix = ducc0_healpix_ang2pix_mex(double(p.Results.nside), double(theta), double(phi), ...
        logical(p.Results.nest), double(p.Results.nthreads));
end
