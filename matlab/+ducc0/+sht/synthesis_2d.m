function map = synthesis_2d(alm, lmax, varargin)
%SYNTHESIS_2D Performs spherical harmonic synthesis (alm2map) for 2D grids
%   MAP = SYNTHESIS_2D(ALM, LMAX) synthesizes a map from spherical harmonic coefficients.
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'mmax', MMAX) sets maximum m order.
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'spin', SPIN) sets spin value (0, 1, or 2).
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'ntheta', NTHETA) sets number of theta rings.
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'nphi', NPHI) sets number of phi pixels per ring.
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'geometry', GEO) sets grid geometry.
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'phi0', PHI0) sets phi offset.
%   MAP = SYNTHESIS_2D(ALM, LMAX, 'nthreads', N) uses N threads.
%
%   Parameters
%   ----------
%   alm : complex array
%       Spherical harmonic coefficients in HEALPix/DUCC ordering.
%       Can be 1D [nalm] or 2D [ncomp, nalm].
%   lmax : int
%       Maximum multipole order l.
%   mmax : int, optional
%       Maximum m order (default: lmax).
%   spin : int, optional
%       Spin value: 0 (scalar), 1 (spin-1), 2 (spin-2) (default: 0).
%   ntheta : int, optional
%       Number of iso-latitude rings (default: determined from geometry and lmax).
%   nphi : int, optional
%       Number of pixels per ring (default: 2*mmax+2).
%   geometry : string, optional
%       Grid geometry: 'CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2' (default: 'CC').
%   phi0 : double, optional
%       Phi offset in radians (default: 0).
%   ringfactor : array, optional
%       Ring factors (size [ntheta]) to multiply each ring.
%   mode : string, optional
%       Transform mode: 'STANDARD', 'GRAD_ONLY', 'DERIV1' (default: 'STANDARD').
%   nthreads : int, optional
%       Number of threads (default: 0 = system default).
%
%   Returns
%   -------
%   map : numeric array
%       Synthesized map on the sphere, shape [nmaps, ntheta, nphi].
%
%   See also: ducc0.sht.analysis_2d, ducc0.sht.adjoint_synthesis_2d

    p = inputParser;
    addRequired(p, 'alm', @(x) isnumeric(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'ntheta', [], @(x) isnumeric(x) && (isscalar(x) || isempty(x)));
    addParameter(p, 'nphi', [], @(x) isnumeric(x) && (isscalar(x) || isempty(x)));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) && (isscalar(x) || isempty(x)));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ringfactor', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'mode', 'STANDARD', @(x) ischar(x) || isstring(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm, lmax, varargin{:});
    
    % Ensure alm is numeric and complex
    if ~isnumeric(alm)
        error('DUCC0:SHT:Synthesis2D:InputError', 'alm must be numeric');
    end
    if isreal(alm)
        error('DUCC0:SHT:Synthesis2D:InputError', 'alm must be complex');
    end
    
    % Convert string to char if needed
    geometry = char(p.Results.geometry);
    mode = char(p.Results.mode);
    
    % Prepare optional parameters
    ntheta = [];
    if ~isempty(p.Results.ntheta)
        ntheta = double(p.Results.ntheta);
    end
    
    nphi = [];
    if ~isempty(p.Results.nphi)
        nphi = double(p.Results.nphi);
    end
    
    mmax = [];
    if ~isempty(p.Results.mmax)
        mmax = double(p.Results.mmax);
    end
    
    ringfactor = [];
    if ~isempty(p.Results.ringfactor)
        ringfactor = double(p.Results.ringfactor(:));
    end
    
    % Call MEX function
    % MEX function expects: (alm, lmax, spin, geometry, ntheta, nphi, mmax, phi0, ringfactor, mode, nthreads)
    map = ducc0_sht_synthesis_2d_mex(alm, double(lmax), double(p.Results.spin), geometry, ...
        ntheta, nphi, mmax, double(p.Results.phi0), ringfactor, mode, double(p.Results.nthreads));
end
