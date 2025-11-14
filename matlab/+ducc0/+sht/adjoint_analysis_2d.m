function map = adjoint_analysis_2d(alm, lmax, varargin)
%ADJOINT_ANALYSIS_2D Performs adjoint spherical harmonic analysis (alm2map)
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX) performs adjoint spherical harmonic
%   analysis (alm2map) for 2D grids. This is the adjoint operation of analysis_2d.
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'spin', SPIN) sets spin value (default: 0).
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'geometry', GEO) sets grid geometry (default: 'CC').
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'ntheta', NTHETA) sets number of theta rings.
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'nphi', NPHI) sets number of phi pixels per ring.
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'mmax', MMAX) sets maximum m order (default: lmax).
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'phi0', PHI0) sets phi offset (default: 0).
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'ringfactor', RF) sets ring factors.
%   MAP = ADJOINT_ANALYSIS_2D(ALM, LMAX, 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   alm : complex array
%       Input spherical harmonic coefficients, shape [ncomp, nalm] or [nalm]
%   lmax : int
%       Maximum multipole order l
%   spin : int, optional
%       Spin value (0, 1, or 2) (default: 0)
%   geometry : string, optional
%       Grid geometry ('CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2') (default: 'CC')
%   ntheta : int, optional
%       Number of theta rings (default: computed from geometry and lmax)
%   nphi : int, optional
%       Number of phi pixels per ring (default: computed from mmax)
%   mmax : int, optional
%       Maximum m order (default: lmax)
%   phi0 : double, optional
%       Phi offset in radians (default: 0)
%   ringfactor : double array, optional
%       Ring factors, size [ntheta] (default: all ones)
%   nthreads : int, optional
%       Number of threads to use (default: 0 = auto)
%
%   Returns
%   -------
%   map : numeric array (real)
%       Synthesized map, shape [nmaps, ntheta, nphi]
%
%   See also: ducc0.sht.analysis_2d, ducc0.sht.adjoint_synthesis_2d

    p = inputParser;
    addRequired(p, 'alm', @(x) isnumeric(x) && ~isreal(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'ntheta', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nphi', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ringfactor', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm, lmax, varargin{:});
    
    if ~isnumeric(alm)
        error('DUCC0:SHT:AdjointAnalysis2D:InputError', 'alm must be numeric');
    end
    if ~iscomplex(alm)
        error('DUCC0:SHT:AdjointAnalysis2D:InputError', 'alm must be complex');
    end
    
    geometry = char(p.Results.geometry);
    
    ntheta = []; if ~isempty(p.Results.ntheta), ntheta = double(p.Results.ntheta); end
    nphi = []; if ~isempty(p.Results.nphi), nphi = double(p.Results.nphi); end
    mmax = []; if ~isempty(p.Results.mmax), mmax = double(p.Results.mmax); end
    ringfactor = []; if ~isempty(p.Results.ringfactor), ringfactor = double(p.Results.ringfactor(:)); end
    
    map = ducc0_sht_adjoint_analysis_2d_mex(alm, double(lmax), double(p.Results.spin), geometry, ...
        ntheta, nphi, mmax, double(p.Results.phi0), ringfactor, double(p.Results.nthreads));
end
