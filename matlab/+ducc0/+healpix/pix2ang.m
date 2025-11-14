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

    error('DUCC0:MEX:NotImplemented', ...
        'pix2ang MEX function is not yet implemented.');

    % TODO: Implement MEX function
end
