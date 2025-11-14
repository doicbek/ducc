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

    error('DUCC0:MEX:NotImplemented', ...
        'ang2pix MEX function is not yet implemented.');

    % TODO: Implement MEX function
end
