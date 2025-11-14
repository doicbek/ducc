function [theta, phi] = pix2ang(nside, pix, varargin)
%PIX2ANG Convert HEALPix pixel index to angular coordinates
%   [THETA, PHI] = PIX2ANG(NSIDE, PIX) converts pixel index to (theta, phi).
%   [THETA, PHI] = PIX2ANG(NSIDE, PIX, 'nest', true) uses NESTED ordering.
%
%   Parameters
%   ----------
%   nside : int
%       HEALPix nside parameter
%   pix : int or int array
%       Pixel index(ices)
%   nest : bool, optional
%       Use NESTED ordering instead of RING (default: false)
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
    addParameter(p, 'nest', false, @islogical);
    parse(p, nside, pix, varargin{:});
    
    py_mod = ducc0.ducc0();
    py_pix = ducc0.util.matlab2numpy(int64(p.Results.pix));
    
    kwargs = py.dict();
    kwargs{'nest'} = p.Results.nest;
    
    py_result = py_mod.healpix.pix2ang(int32(p.Results.nside), py_pix, pyargs(kwargs));
    
    % Extract theta and phi from tuple
    theta = ducc0.util.numpy2matlab(py_result{1});
    phi = ducc0.util.numpy2matlab(py_result{2});
end

