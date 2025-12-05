function map = synthesis(alm, lmax, spin, theta, nphi, phi0, ringstart, varargin)
%SYNTHESIS Performs spherical harmonic synthesis (alm2map) for arbitrary theta rings
%   MAP = SYNTHESIS(ALM, LMAX, SPIN, THETA, NPHI, PHI0, RINGSTART) synthesizes
%   a map from spherical harmonic coefficients using arbitrary theta rings.
%   MAP = SYNTHESIS(..., 'mmax', MMAX) sets maximum m order (default: lmax).
%   MAP = SYNTHESIS(..., 'mstart', MSTART) sets mstart array.
%   MAP = SYNTHESIS(..., 'lstride', LSTRIDE) sets l stride (default: 1).
%   MAP = SYNTHESIS(..., 'pixstride', PIXSTRIDE) sets pixel stride (default: 1).
%   MAP = SYNTHESIS(..., 'ringfactor', RF) sets ring factors.
%   MAP = SYNTHESIS(..., 'mode', MODE) sets transform mode (default: 'STANDARD').
%   MAP = SYNTHESIS(..., 'theta_interpol', TF) uses theta interpolation (default: false).
%   MAP = SYNTHESIS(..., 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   alm : complex array
%       Spherical harmonic coefficients, shape [ncomp, nalm] or [nalm]
%   lmax : int
%       Maximum multipole order l
%   spin : int
%       Spin value (0, 1, or 2)
%   theta : double array, size [nrings]
%       Colatitudes of map rings in radians
%   nphi : int array, size [nrings]
%       Number of pixels per ring
%   phi0 : double array, size [nrings]
%       Azimuth of first pixel in each ring in radians
%   ringstart : int array, size [nrings]
%       Index in map where first pixel of each ring is stored
%   mmax : int, optional
%       Maximum m order (default: lmax)
%   mstart : int array, optional
%       mstart array, size [mmax+1] (default: computed from lmax, mmax)
%   lstride : int, optional
%       Stride in alm between l and l+1 (default: 1)
%   pixstride : int, optional
%       Stride in map between pixels (default: 1)
%   ringfactor : double array, optional
%       Ring factors, size [nrings] (default: all ones)
%   mode : string, optional
%       Transform mode: 'STANDARD', 'GRAD_ONLY', 'DERIV1' (default: 'STANDARD')
%   theta_interpol : bool, optional
%       Use theta interpolation for irregular grids (default: false)
%   nthreads : int, optional
%       Number of threads (default: 0 = system default)
%
%   Returns
%   -------
%   map : numeric array
%       Synthesized map, shape [nmaps, npix]
%
%   See also: ducc0.sht.synthesis_2d, ducc0.sht.adjoint_synthesis

    p = inputParser;
    addRequired(p, 'alm', @(x) isnumeric(x) && ~isreal(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'spin', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'theta', @(x) isnumeric(x) && isvector(x));
    addRequired(p, 'nphi', @(x) isnumeric(x) && isvector(x));
    addRequired(p, 'phi0', @(x) isnumeric(x) && isvector(x));
    addRequired(p, 'ringstart', @(x) isnumeric(x) && isvector(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'mstart', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'lstride', 1, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'pixstride', 1, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ringfactor', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'mode', 'STANDARD', @(x) ischar(x) || isstring(x));
    addParameter(p, 'theta_interpol', false, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'N_batch', 1, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm, lmax, spin, theta, nphi, phi0, ringstart, varargin{:});
    
    % Ensure inputs are correct types
    if isreal(alm)
        error('DUCC0:SHT:Synthesis:InputError', 'alm must be complex');
    end
    
    N_batch = p.Results.N_batch;
    
    % Support both 2D [ncomp, nalm] and 3D [N, ncomp, nalm] arrays
    if ndims(alm) ~= 2 && ndims(alm) ~= 3
        error('DUCC0:SHT:Synthesis:InputError', 'alm must be 2D array [ncomp, nalm] or 3D array [N, ncomp, nalm]');
    end
    
    % Convert to column vectors and ensure correct types
    theta = double(theta(:));
    nphi = double(nphi(:));
    phi0 = double(phi0(:));
    ringstart = double(ringstart(:));
    
    % Check sizes match
    nrings = length(theta);
    if length(nphi) ~= nrings || length(phi0) ~= nrings || length(ringstart) ~= nrings
        error('DUCC0:SHT:Synthesis:InputError', ...
            'theta, nphi, phi0, and ringstart must have the same size');
    end
    
    % Prepare optional parameters
    mmax = [];
    if ~isempty(p.Results.mmax)
        mmax = double(p.Results.mmax);
    end
    
    mstart = [];
    if ~isempty(p.Results.mstart)
        mstart = double(p.Results.mstart(:));
    end
    
    ringfactor = [];
    if ~isempty(p.Results.ringfactor)
        ringfactor = double(p.Results.ringfactor(:));
        if length(ringfactor) ~= nrings
            error('DUCC0:SHT:Synthesis:InputError', ...
                'ringfactor size must match nrings');
        end
    end
    
    % Convert string to char if needed
    mode = char(p.Results.mode);
    theta_interpol = logical(p.Results.theta_interpol);
    
    % Call MEX function
    map = ducc0_sht_synthesis_mex(alm, double(lmax), double(p.Results.spin), ...
        theta, nphi, phi0, ringstart, ...
        mmax, mstart, double(p.Results.lstride), double(p.Results.pixstride), ...
        ringfactor, mode, theta_interpol, double(p.Results.nthreads));
end

