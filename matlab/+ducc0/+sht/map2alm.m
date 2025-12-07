function alm = map2alm(map, spin, map_info, alm_info, varargin)
%MAP2ALM Computes spherical harmonic transform (SHT) from map to alm
%   ALM = MAP2ALM(MAP, SPIN, MAP_INFO, ALM_INFO) computes the spherical
%   harmonic transform (SHT) for input maps using DUCC SHT mex modules.
%   ALM = MAP2ALM(..., 'n_iter', N) sets number of Jacobi iterations (default: 3).
%   ALM = MAP2ALM(..., 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
   %   map : numeric array (real), dense or sparse
   %       Input maps. Can be either:
   %       - Single map: shape [nmaps, npix] where nmaps is either 1 (for spin-0)
   %         or 2 (for spin>0 fields), and npix is the number of pixels.
   %       - Multiple maps: shape [N, ncomp*npix] where N is the number of maps,
   %         ncomp is 1 (spin-0) or 2 (spin>0), and npix is the number of pixels.
   %       Sparse arrays are supported and sparsity is preserved throughout.
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
%       Harmonic coefficients a_lm of the input map(s).
%       A set of two arrays (E and B modes) is returned if spin>0.
%       For single map: shape [nmaps, nalm] where nalm = nelem from alm_info.
%       For multiple maps: shape [N, ncomp*nalm] where N is the number of maps.
%
%   Example
%   -------
%   nside = 256;
%   map_info = ducc0.sht.create_map_info(nside);
%   alm_info = ducc0.sht.create_alm_info(3*nside-1);
%   map = randn(1, map_info.npix);
%   alm = ducc0.sht.map2alm(map, 0, map_info, alm_info);
%
%   % Multiple maps example:
%   N = 10;
%   ncomp = 1;
%   maps = randn(N, ncomp * map_info.npix);
%   alms = ducc0.sht.map2alm(maps, 0, map_info, alm_info);
%
%   See also: alm2map, create_map_info, create_alm_info

    p = inputParser;
    addRequired(p, 'map', @(x) isnumeric(x) && isreal(x));  % Accepts both dense and sparse
    addRequired(p, 'spin', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'map_info', @(x) isstruct(x));
    addRequired(p, 'alm_info', @(x) isstruct(x));
    addParameter(p, 'n_iter', 3, @(x) isnumeric(x) && isscalar(x) && x >= 0);
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, map, spin, map_info, alm_info, varargin{:});
    
    % Detect if input is sparse
    is_sparse_input = issparse(map);
    
    n_iter = int32(p.Results.n_iter);
    nthreads = int32(p.Results.nthreads);
    spin = int32(p.Results.spin);
    
    % Determine ncomp based on spin
    if spin == 0
        ncomp = 1;
    else
        ncomp = 2;
    end
    
    % Get expected npix from map_info
    npix_expected = map_info.npix;
    
    % Detect input format: [N, ncomp*npix] (batch) vs [ncomp, npix] (single)
    map = squeeze(map);
    map_dims = size(map);
    
    is_batch_mode = false;
    if length(map_dims) == 2
        % Check if second dimension matches ncomp*npix (batch mode)
        if map_dims(2) == ncomp * npix_expected && map_dims(1) > 1
            is_batch_mode = true;
            N = map_dims(1);
        elseif map_dims(1) == ncomp && map_dims(2) == npix_expected
            % Single map format [ncomp, npix]
            is_batch_mode = false;
            N = 1;
        elseif isvector(map) && length(map) == npix_expected
            % Single map as vector [1, npix] for spin-0
            map = map(:)';  % Make it [1, npix]
            is_batch_mode = false;
            N = 1;
        else
            error('DUCC0:SHT:Map2Alm:InputError', ...
                'Map dimensions do not match expected format. Expected [ncomp, npix] or [N, ncomp*npix]');
        end
    else
        error('DUCC0:SHT:Map2Alm:InputError', ...
            'Map must be 2D array');
    end
    
    % Reshape map to [N, ncomp, npix] for consistent processing
    if is_batch_mode
        % Reshape from [N, ncomp*npix] to [N, ncomp, npix]
        map = reshape(map, [N, ncomp, npix_expected]);
    else
        % Reshape from [ncomp, npix] to [1, ncomp, npix]
        map = reshape(map, [1, ncomp, npix_expected]);
        N = 1;
    end
    
    % Pad map if CAR pixelization
    % Need to handle each map separately for padding
    map_padded = [];
    for i = 1:N
        % Extract single map [ncomp, npix] without removing dimensions
        map_single = reshape(map(i, :, :), [ncomp, npix_expected]);
        map_single_padded = ducc0.sht.pad_map(map_single, map_info.si);
        if i == 1
            % Initialize with correct size after padding
            npix_padded = size(map_single_padded, 2);
            % Preserve sparsity in initialization
            if is_sparse_input
                map_padded = sparse(N, ncomp, npix_padded);
            else
                map_padded = zeros(N, ncomp, npix_padded);
            end
        end
        map_padded(i, :, :) = map_single_padded;
    end
    map = map_padded;
    npix = size(map, 3);
    
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
    % Preserve sparsity in initialization
    if is_sparse_input
        map_weighted = sparse(size(map));
    else
        map_weighted = zeros(size(map));
    end
    for i = 1:N
        % Extract single map [ncomp, npix] without removing dimensions
        map_single = reshape(map(i, :, :), [ncomp, size(map, 3)]);
        map_weighted(i, :, :) = ducc0.sht.times_weight(map_single, sht_info);
    end
    
    % First iteration: adjoint synthesis (batch mode)
    % map_weighted is [N, ncomp, npix] which matches batch mode input
    alm = ducc0.sht.adjoint_synthesis(map_weighted, lmax, spin, theta, nphi, ...
        phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
        'nthreads', nthreads, 'N_batch', N);
    
    % Iterative refinement using Jacobi iterations
    for i = 1:n_iter
        % Compute residual: map - synthesis(alm)
        map_synth = ducc0.sht.synthesis(alm, lmax, spin, theta, nphi, ...
            phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
            'nthreads', nthreads, 'N_batch', N);
        
        % Compute difference
        dmap = map_weighted - map_synth;
        
        % Update alm with adjoint of residual
        dalm = ducc0.sht.adjoint_synthesis(dmap, lmax, spin, theta, nphi, ...
            phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
            'nthreads', nthreads, 'N_batch', N);
        
        % Subtract correction
        alm = alm - dalm;
    end
    
    % Reshape output: if single map, return [ncomp, nalm], else [N, ncomp*nalm]
    if N == 1
        alm = squeeze(alm);  % Remove singleton dimension
    else
        % Reshape from [N, ncomp, nalm] to [N, ncomp*nalm]
        [~, ncomp_out, nalm] = size(alm);
        alm = reshape(alm, [N, ncomp_out * nalm]);
    end
end

