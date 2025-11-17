function alm_out = rotate_alm(alm_in, lmax, psi, theta, phi, varargin)
%ROTATE_ALM Rotates spherical harmonic coefficients according to Euler angles
%   ALM_OUT = ROTATE_ALM(ALM_IN, LMAX, PSI, THETA, PHI) rotates spherical
%   harmonic coefficients according to Euler angles (psi, theta, phi).
%   ALM_OUT = ROTATE_ALM(ALM_IN, LMAX, PSI, THETA, PHI, 'mmax_in', MMAX_IN)
%   sets maximum m order in input (default: lmax).
%   ALM_OUT = ROTATE_ALM(ALM_IN, LMAX, PSI, THETA, PHI, 'mmax_out', MMAX_OUT)
%   sets maximum m order in output (default: lmax).
%   ALM_OUT = ROTATE_ALM(ALM_IN, LMAX, PSI, THETA, PHI, 'nthreads', N) uses
%   N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   alm_in : complex array
%       Input spherical harmonic coefficients, shape [ncomp, nalm] or [nalm]
%   lmax : int
%       Maximum multipole order l
%   psi : double
%       First rotation angle about z-axis (radians)
%   theta : double
%       Second rotation angle about y-axis (radians)
%   phi : double
%       Third rotation angle about z-axis (radians)
%   mmax_in : int, optional
%       Maximum m order in input (default: lmax)
%   mmax_out : int, optional
%       Maximum m order in output (default: lmax)
%   nthreads : int, optional
%       Number of threads to use (default: 0 = auto)
%
%   Returns
%   -------
%   alm_out : complex array
%       Rotated spherical harmonic coefficients, same shape as alm_in
%
%   See also: ducc0.sht.synthesis_2d, ducc0.sht.analysis_2d

    p = inputParser;
    addRequired(p, 'alm_in', @(x) isnumeric(x) && ~isreal(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'psi', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'theta', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'phi', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'mmax_in', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'mmax_out', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm_in, lmax, psi, theta, phi, varargin{:});
    
    mmax_in = []; if ~isempty(p.Results.mmax_in), mmax_in = double(p.Results.mmax_in); end
    mmax_out = []; if ~isempty(p.Results.mmax_out), mmax_out = double(p.Results.mmax_out); end
    
    alm_out = ducc0_sht_rotate_alm_mex(alm_in, double(lmax), double(psi), double(theta), double(phi), ...
        mmax_in, mmax_out, double(p.Results.nthreads));
end



