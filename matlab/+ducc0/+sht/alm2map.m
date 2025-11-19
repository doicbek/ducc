function map = alm2map(alm, spin, map_info, alm_info, varargin)
%ALM2MAP Computes inverse spherical harmonic transform (SHT) from alm to map
%   MAP = ALM2MAP(ALM, SPIN, MAP_INFO, ALM_INFO) computes the inverse
%   spherical harmonic transform (SHT) for input alm coefficients using
%   DUCC SHT mex modules.
%   MAP = ALM2MAP(..., 'nthreads', N) uses N threads (default: 0 = auto).
%
%   Parameters
%   ----------
%   alm : complex array
%       Spherical harmonic coefficients, shape [nmaps, nalm] where nmaps is
%       either 1 (for spin-0 fields) or 2 (for spin-s fields), and nalm is
%       the number of harmonic coefficients (nelem from alm_info).
%   spin : int
%       Field spin (0, 1, or 2)
%   map_info : struct
%       Map information structure from create_map_info (HEALPix only)
%   alm_info : struct
%       Alm information structure from create_alm_info
%   nthreads : int, optional
%       Number of threads (default: 0 = system default)
%
%   Returns
%   -------
%   map : numeric array (real)
%       Map reconstructed from the input a_lm coefficients.
%       A set of two arrays (e.g. Q and U Stokes parameters) is returned
%       if spin>0. Shape [nmaps, npix] where npix comes from map_info.
%
%   Example
%   -------
%   nside = 256;
%   lmax = 3*nside - 1;
%   map_info = ducc0.sht.create_map_info(nside);
%   alm_info = ducc0.sht.create_alm_info(lmax);
%   nalm = alm_info.nelem;
%   alm = randn(1, nalm) + 1i*randn(1, nalm);
%   map = ducc0.sht.alm2map(alm, 0, map_info, alm_info);
%
%   See also: map2alm, create_map_info, create_alm_info

    p = inputParser;
    addRequired(p, 'alm', @(x) isnumeric(x));
    addRequired(p, 'spin', @(x) isnumeric(x) && isscalar(x));
    addRequired(p, 'map_info', @(x) isstruct(x));
    addRequired(p, 'alm_info', @(x) isstruct(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm, spin, map_info, alm_info, varargin{:});
    
    nthreads = int32(p.Results.nthreads);
    spin = int32(p.Results.spin);
    
    % Ensure alm is 2D: [nmaps, nalm]
    alm = squeeze(alm);
    if isvector(alm)
        alm = alm(:)';  % Make it [1, nalm]
    end
    
    % Check if alm is complex
    if isreal(alm)
        warning('DUCC0:SHT:Alm2Map:InputWarning', ...
            'alm should be complex, converting to complex');
        alm = complex(alm);
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
    
    % Perform synthesis (alm2map)
    map = ducc0.sht.synthesis(alm, lmax, spin, theta, nphi, ...
        phi0, ringstart, 'mmax', mmax, 'mstart', mstart, ...
        'nthreads', nthreads);
    
    % Unpad map if CAR pixelization
    map = ducc0.sht.unpad_map(map, sht_info);
end

