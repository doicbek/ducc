function alm = map2alm(map, spin, map_info, alm_info, varargin)
%MAP2ALM Computes spherical harmonic transform (SHT) from map to alm
%   ALM = MAP2ALM(MAP, SPIN, MAP_INFO, ALM_INFO) computes the spherical
%   harmonic transform (SHT) for input maps using DUCC SHT mex modules.
%   ALM = MAP2ALM(..., 'n_iter', N) sets number of Jacobi iterations (default: 3).
%   ALM = MAP2ALM(..., 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   map : numeric array (real)
%       Input maps, shape [nmaps, npix] where nmaps is either 1 (for spin-0)
%       or 2 (for spin>0 fields), and npix is the number of pixels.
%   spin : int
%       Field spin (0, 1, or 2)
%   map_info : struct
%       Map information structure from create_map_info (HEALPix only)
%   alm_info : struct
%       Alm information structure from create_alm_info
%   n_iter : int, optional
%       Number of Jacobi iterations used to improve SHT accuracy (default: 3)
%   nthreads : int, optional
%       Number of threads (default: 0 = system default)
%
%   Returns
%   -------
%   alm : complex array
%       Harmonic coefficients a_lm of the input map.
%       A set of two arrays (E and B modes) is returned if spin>0.
%       Shape [nmaps, nalm] where nalm = nelem from alm_info.
%
%   Example
%   -------
%   nside = 256;
%   map_info = ducc0.sht.create_map_info(nside);
%   alm_info = ducc0.sht.create_alm_info(3*nside-1);
%   map = randn(1, map_info.npix);
%   alm = ducc0.sht.map2alm(map, 0, map_info, alm_info);
%
%   See also: alm2map, create_map_info, create_alm_info

    p = inputParser;
    addRequired(p, 'map', @(x) isnumeric(x) && isreal(x));
    addRequired(p, 'spin', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'map_info', @(x) isstruct(x));
    addRequired(p, 'alm_info', @(x) isstruct(x));
    addParameter(p, 'n_iter', 3, @(x) isnumeric(x) && isscalar(x) && x >= 0);
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, map, spin, map_info, alm_info, varargin{:});
    
    n_iter = int32(p.Results.n_iter);
    nthreads = int32(p.Results.nthreads);
    spin = int32(p.Results.spin);
    
    % Ensure map is 2D: [nmaps, npix]
    map = squeeze(map);
    if isvector(map)
        map = map(:)';  % Make it [1, npix]
    end
    
    % Pad map if CAR pixelization
    map = ducc0.sht.pad_map(map, map_info.si);
    
    % Extract SHT info
    sht_info = map_info.si;
    theta = sht_info.theta;
    nphi = double(sht_info.nphi);
    phi0 = sht_info.phi0;
    ringstart = double(sht_info.ringstart);
    
    % Prepare alm parameters
    lmax = alm_info.lmax;
    mmax = alm_info.mmax;
    mstart = double(alm_info.mstart);
    
    % Convert ring weights to ringfactor if needed
    % For map2alm (adjoint_synthesis), we need to multiply by weights
    % The weights are applied via times_weight before adjoint_synthesis
    map_weighted = ducc0.sht.times_weight(map, sht_info);
    
    % First iteration: adjoint synthesis
    alm = ducc0.sht.adjoint_synthesis(map_weighted, lmax, spin, theta, nphi, ...
        phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
        'nthreads', nthreads);
    
    % Iterative refinement using Jacobi iterations
    for i = 1:n_iter
        % Compute residual: map - synthesis(alm)
        map_synth = ducc0.sht.synthesis(alm, lmax, spin, theta, nphi, ...
            phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
            'nthreads', nthreads);
        
        % Compute difference
        dmap = map - map_synth;
        
        % Update alm with adjoint of residual
        dmap_weighted = ducc0.sht.times_weight(dmap, sht_info);
        dalm = ducc0.sht.adjoint_synthesis(dmap_weighted, lmax, spin, theta, nphi, ...
            phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
            'nthreads', nthreads);
        
        % Subtract correction
        alm = alm - dalm;
    end
end

