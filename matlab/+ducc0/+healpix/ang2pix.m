function pix = ang2pix(nside, theta, phi, varargin)
%ANG2PIX Convert angular coordinates to HEALPix pixel index
%   PIX = ANG2PIX(NSIDE, THETA, PHI) converts (theta, phi) to pixel index.
%   PIX = ANG2PIX(NSIDE, THETA, PHI, 'nest', true) uses NESTED ordering.
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
%
%   Returns
%   -------
%   pix : int or int array
%       Pixel index(ices)

    p = inputParser;
    addRequired(p, 'nside', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'theta', @(x) isnumeric(x));
    addRequired(p, 'phi', @(x) isnumeric(x));
    addParameter(p, 'nest', false, @islogical);
    parse(p, nside, theta, phi, varargin{:});
    
    py_mod = ducc0.ducc0();
    py_theta = ducc0.util.matlab2numpy(p.Results.theta);
    py_phi = ducc0.util.matlab2numpy(p.Results.phi);
    
    kwargs = py.dict();
    kwargs{'nest'} = p.Results.nest;
    
    py_result = py_mod.healpix.ang2pix(int32(p.Results.nside), py_theta, py_phi, pyargs(kwargs));
    pix = ducc0.util.numpy2matlab(py_result);
    pix = int64(pix);  % Pixel indices should be integers
end

