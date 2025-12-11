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
    
    % For sparse arrays, keep in 2D format (MATLAB doesn't support 3D sparse)
    % For dense arrays, reshape to 3D for easier processing
    if is_sparse_input
        % Keep sparse arrays in 2D format
        if is_batch_mode
            % Keep as [N, ncomp*npix] - will be converted to (N*ncomp) x npix for MEX
            % No reshaping needed
        else
            % Keep as [ncomp, npix] for single map
            % Reshape single vector to [ncomp, npix] if needed
            if ~is_batch_mode && map_dims(1) ~= ncomp
                % Already handled above, but ensure shape is correct
            end
        end
        npix = npix_expected;
        % Pad map if CAR pixelization (handle sparse arrays in 2D)
        % For HEALPix, pad_map is a no-op and preserves sparsity
        if is_batch_mode
            % For batch mode sparse: [N, ncomp*npix]
            % Process each map separately to handle padding
            % First check if padding is needed by testing first map
            map_first_2d = reshape(map(1, :), [ncomp, npix_expected]);
            map_first_padded = ducc0.sht.pad_map(map_first_2d, map_info.si);
            npix_padded = size(map_first_padded, 2);
            
            if npix_padded ~= npix_expected
                % Need padding - rebuild sparse array with padding
                map_padded = sparse(N, ncomp * npix_padded);
                % Use parfor for parallelization when N is large enough
                % Threshold chosen to balance overhead vs. benefit
                use_parfor = (N > 100) && (exist('parfor', 'builtin') == 5);
                if use_parfor
                    % Pre-allocate temporary storage for parallel loop
                    map_rows_cell = cell(N, 1);
                    parfor i = 1:N
                        % Extract map i and reshape to [ncomp, npix]
                        map_row = map(i, :);
                        % For sparse arrays, need to extract as full for reshaping
                        % (unavoidable when padding is needed)
                        map_2d = reshape(full(map_row), [ncomp, npix_expected]);
                        % Pad (preserves structure, may return sparse if input was sparse)
                        map_2d_padded = ducc0.sht.pad_map(map_2d, map_info.si);
                        % Reshape back to row and store in cell array
                        map_rows_cell{i} = sparse(reshape(map_2d_padded, [1, ncomp * npix_padded]));
                    end
                    % Copy results back to sparse array
                    for i = 1:N
                        map_padded(i, :) = map_rows_cell{i};
                    end
                else
                    % Sequential processing for small N or when parfor unavailable
                    for i = 1:N
                        % Extract map i and reshape to [ncomp, npix]
                        map_row = map(i, :);
                        % For sparse arrays, need to extract as full for reshaping
                        % (unavoidable when padding is needed)
                        map_2d = reshape(full(map_row), [ncomp, npix_expected]);
                        % Pad (preserves structure, may return sparse if input was sparse)
                        map_2d_padded = ducc0.sht.pad_map(map_2d, map_info.si);
                        % Reshape back to row and store as sparse
                        map_padded(i, :) = sparse(reshape(map_2d_padded, [1, ncomp * npix_padded]));
                    end
                end
                map = map_padded;
                npix = npix_padded;
            end
            % If no padding needed, map stays as-is (sparse preserved)
        else
            % Single map: [ncomp, npix]
            % pad_map preserves sparsity (no-op for HEALPix)
            map_padded = ducc0.sht.pad_map(map, map_info.si);
            map = map_padded;
            npix = size(map, 2);
        end
    else
        % Dense arrays: reshape to 3D for easier processing
        if is_batch_mode
            % Reshape from [N, ncomp*npix] to [N, ncomp, npix]
            map = reshape(map, [N, ncomp, npix_expected]);
        else
            % Reshape from [ncomp, npix] to [1, ncomp, npix]
            map = reshape(map, [1, ncomp, npix_expected]);
            N = 1;
        end
        
        % Pad map if CAR pixelization
        % First determine padded size by checking first map
        map_first = reshape(map(1, :, :), [ncomp, npix_expected]);
        map_first_padded = ducc0.sht.pad_map(map_first, map_info.si);
        npix_padded = size(map_first_padded, 2);
        map_padded = zeros(N, ncomp, npix_padded);
        
        % Use parfor for parallelization when N is large enough
        use_parfor = (N > 100) && (exist('parfor', 'builtin') == 5);
        if use_parfor
            parfor i = 1:N
                % Extract single map [ncomp, npix] without removing dimensions
                map_single = reshape(map(i, :, :), [ncomp, npix_expected]);
                map_single_padded = ducc0.sht.pad_map(map_single, map_info.si);
                map_padded(i, :, :) = map_single_padded;
            end
        else
            % Sequential processing for small N or when parfor unavailable
            for i = 1:N
                % Extract single map [ncomp, npix] without removing dimensions
                map_single = reshape(map(i, :, :), [ncomp, npix_expected]);
                map_single_padded = ducc0.sht.pad_map(map_single, map_info.si);
                map_padded(i, :, :) = map_single_padded;
            end
        end
        map = map_padded;
        npix = size(map, 3);
    end
    
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
    if is_sparse_input
        % For sparse arrays, multiply directly (preserves sparsity)
        % map is 2D: [N, ncomp*npix] or [ncomp, npix]
        map_weighted = ducc0.sht.times_weight(map, sht_info);
    else
        % For dense arrays, use vectorized operation
        % times_weight multiplies by a scalar weight, which works element-wise on 3D arrays
        % Reshape to 2D, multiply, then reshape back
        [N_dim, ncomp_dim, npix_dim] = size(map);
        map_2d = reshape(map, [N_dim * ncomp_dim, npix_dim]);
        map_weighted_2d = ducc0.sht.times_weight(map_2d, sht_info);
        map_weighted = reshape(map_weighted_2d, [N_dim, ncomp_dim, npix_dim]);
    end
    
    % First iteration: adjoint synthesis (batch mode)
    % For sparse arrays, map_weighted is 2D [N, ncomp*npix] or [ncomp, npix]
    % The MEX function will handle the 2D sparse format correctly
    alm = ducc0.sht.adjoint_synthesis(map_weighted, lmax, spin, theta, nphi, ...
        phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
        'nthreads', nthreads, 'N_batch', N);
    
    % Iterative refinement using Jacobi iterations
    for i = 1:n_iter
        % Compute residual: map - synthesis(alm)
        map_synth = ducc0.sht.synthesis(alm, lmax, spin, theta, nphi, ...
            phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
            'nthreads', nthreads, 'N_batch', N);
        
        % Compute difference (preserves sparsity if map_weighted is sparse)
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

