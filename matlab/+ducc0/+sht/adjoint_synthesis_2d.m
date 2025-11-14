function alm = adjoint_synthesis_2d(map, lmax, varargin)
%ADJOINT_SYNTHESIS_2D Performs adjoint spherical harmonic synthesis (map2alm)
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX) performs adjoint spherical harmonic
%   synthesis (map2alm) for 2D grids. This is the adjoint operation of synthesis_2d.
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'spin', SPIN) sets spin value (default: 0).
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'geometry', GEO) sets grid geometry (default: 'CC').
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'mmax', MMAX) sets maximum m order (default: lmax).
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'phi0', PHI0) sets phi offset (default: 0).
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'ringfactor', RF) sets ring factors.
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'mode', MODE) sets transform mode (default: 'STANDARD').
%   ALM = ADJOINT_SYNTHESIS_2D(MAP, LMAX, 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   map : numeric array (real)
%       Input map data, shape [nmaps, ntheta, nphi]
%   lmax : int
%       Maximum multipole order l
%   spin : int, optional
%       Spin value (0, 1, or 2) (default: 0)
%   geometry : string, optional
%       Grid geometry ('CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2') (default: 'CC')
%   mmax : int, optional
%       Maximum m order (default: lmax)
%   phi0 : double, optional
%       Phi offset in radians (default: 0)
%   ringfactor : double array, optional
%       Ring factors, size [ntheta] (default: all ones)
%   mode : string, optional
%       Transform mode ('STANDARD', 'GRAD_ONLY', 'DERIV1') (default: 'STANDARD')
%   nthreads : int, optional
%       Number of threads to use (default: 0 = auto)
%
%   Returns
%   -------
%   alm : complex array
%       Spherical harmonic coefficients, shape [ncomp, nalm]
%
%   See also: ducc0.sht.synthesis_2d, ducc0.sht.adjoint_analysis_2d

    p = inputParser;
    addRequired(p, 'map', @(x) isnumeric(x) && isreal(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ringfactor', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'mode', 'STANDARD', @(x) ischar(x) || isstring(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, map, lmax, varargin{:});
    
    if ~isnumeric(map)
        error('DUCC0:SHT:AdjointSynthesis2D:InputError', 'map must be numeric');
    end
    if ~isreal(map)
        error('DUCC0:SHT:AdjointSynthesis2D:InputError', 'map must be real');
    end
    if ndims(map) ~= 3
        error('DUCC0:SHT:AdjointSynthesis2D:InputError', 'map must be 3D array [nmaps, ntheta, nphi]');
    end
    
    geometry = char(p.Results.geometry);
    mode = char(p.Results.mode);
    
    mmax = []; if ~isempty(p.Results.mmax), mmax = double(p.Results.mmax); end
    ringfactor = []; if ~isempty(p.Results.ringfactor), ringfactor = double(p.Results.ringfactor(:)); end
    
    alm = ducc0_sht_adjoint_synthesis_2d_mex(map, double(lmax), double(p.Results.spin), geometry, ...
        mmax, double(p.Results.phi0), ringfactor, mode, double(p.Results.nthreads));
end
