function alm = analysis_2d(map, lmax, varargin)
%ANALYSIS_2D Performs spherical harmonic analysis (map2alm) for 2D grids
%   ALM = ANALYSIS_2D(MAP, LMAX) analyzes a map into spherical harmonic coefficients.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'mmax', MMAX) sets maximum m order.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'spin', SPIN) sets spin value (0, 1, or 2).
%   ALM = ANALYSIS_2D(MAP, LMAX, 'geometry', GEO) sets grid geometry.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'phi0', PHI0) sets phi offset.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'nthreads', N) uses N threads.
%
%   Parameters
%   ----------
%   map : numeric array
%       Map data, shape [nmaps, ntheta, nphi].
%       nmaps = 1 for spin=0, nmaps = 2 for spin>0.
%   lmax : int
%       Maximum multipole order l.
%   mmax : int, optional
%       Maximum m order (default: lmax).
%   spin : int, optional
%       Spin value: 0 (scalar), 1 (spin-1), 2 (spin-2) (default: 0).
%   geometry : string, optional
%       Grid geometry: 'CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2' (default: 'CC').
%   phi0 : double, optional
%       Phi offset in radians (default: 0).
%   ringfactor : array, optional
%       Ring factors (size [ntheta]) to multiply each ring.
%   nthreads : int, optional
%       Number of threads (default: 0 = system default).
%
%   Returns
%   -------
%   alm : complex array
%       Spherical harmonic coefficients, shape [ncomp, nalm].
%
%   This is the inverse operation of synthesis_2d.
%
%   See also: ducc0.sht.synthesis_2d, ducc0.sht.adjoint_analysis_2d

    p = inputParser;
    addRequired(p, 'map', @(x) isnumeric(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) && (isscalar(x) || isempty(x)));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ringfactor', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, map, lmax, varargin{:});
    
    % Ensure map is numeric and real
    if ~isnumeric(map)
        error('DUCC0:SHT:Analysis2D:InputError', 'map must be numeric');
    end
    if ~isreal(map)
        error('DUCC0:SHT:Analysis2D:InputError', 'map must be real');
    end
    
    % Check dimensions
    if ndims(map) ~= 3
        error('DUCC0:SHT:Analysis2D:InputError', 'map must be 3D array [nmaps, ntheta, nphi]');
    end
    
    % Convert string to char if needed
    geometry = char(p.Results.geometry);
    
    % Prepare optional parameters
    mmax = [];
    if ~isempty(p.Results.mmax)
        mmax = double(p.Results.mmax);
    end
    
    ringfactor = [];
    if ~isempty(p.Results.ringfactor)
        ringfactor = double(p.Results.ringfactor(:));
    end
    
    % Call MEX function
    % MEX function expects: (map, lmax, spin, geometry, mmax, phi0, ringfactor, nthreads)
    alm = ducc0_sht_analysis_2d_mex(map, double(lmax), double(p.Results.spin), geometry, ...
        mmax, double(p.Results.phi0), ringfactor, double(p.Results.nthreads));
end
