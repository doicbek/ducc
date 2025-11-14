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
%   lmax : int
%       Maximum multipole order l.
%   mmax : int, optional
%       Maximum m order (default: lmax).
%   spin : int, optional
%       Spin value: 0 (scalar), 1 (spin-1), 2 (spin-2) (default: 0).
%   ntheta : int, optional
%       Number of iso-latitude rings (default: lmax+1).
%   nphi : int, optional
%       Number of pixels per ring (default: 2*mmax+2).
%   geometry : string, optional
%       Grid geometry: 'CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2' (default: 'CC').
%   phi0 : double, optional
%       Phi offset in radians (default: 0).
%   nthreads : int, optional
%       Number of threads (default: 0 = system default).
%
%   Returns
%   -------
%   map : numeric array
%       Synthesized map on the sphere.
%
%   See also: ducc0.sht.analysis_2d, ducc0.sht.adjoint_synthesis_2d

    p = inputParser;
    addRequired(p, 'alm', @(x) isnumeric(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ntheta', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nphi', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm, lmax, varargin{:});
    
    py_alm = ducc0.util.matlab2numpy(p.Results.alm);
    kwargs = py.dict();
    kwargs{'lmax'} = int32(p.Results.lmax);
    if ~isempty(p.Results.mmax)
        kwargs{'mmax'} = int32(p.Results.mmax);
    end
    kwargs{'spin'} = int32(p.Results.spin);
    if ~isempty(p.Results.ntheta)
        kwargs{'ntheta'} = int32(p.Results.ntheta);
    end
    if ~isempty(p.Results.nphi)
        kwargs{'nphi'} = int32(p.Results.nphi);
    end
    kwargs{'geometry'} = char(p.Results.geometry);
    kwargs{'phi0'} = double(p.Results.phi0);
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.sht.synthesis_2d(py_alm, pyargs(kwargs));
    map = ducc0.util.numpy2matlab(py_result);
end

