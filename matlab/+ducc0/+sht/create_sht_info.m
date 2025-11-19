function sht_info = create_sht_info(map_info)
%CREATE_SHT_INFO Create SHT info structure from HEALPix map info
%   SHT_INFO = CREATE_SHT_INFO(MAP_INFO) creates an SHT info structure
%   containing ring information for spherical harmonic transforms.
%
%   This is a helper function used by map2alm and alm2map.
%
%   See also: create_map_info, create_alm_info

    if ~map_info.is_healpix
        error('DUCC0:SHT:CreateShtInfo:InputError', ...
            'Only HEALPix pixelization is supported');
    end
    
    % HEALPix pixelization
    nside = map_info.nside;
    npix = 12*nside*nside;
    rings = (1:(4*nside-1))';
    nring = length(rings);
    
    theta = zeros(nring, 1, 'double');
    phi0 = zeros(nring, 1, 'double');
    nphi = zeros(nring, 1, 'uint64');
    
    northrings = rings;
    northrings(rings > 2*nside) = 4*nside - rings(rings > 2*nside);
    
    % Handle polar cap
    cap = northrings < nside;
    theta(cap) = 2*asin(northrings(cap)/(sqrt(6)*nside));
    nphi(cap) = 4*northrings(cap);
    phi0(cap) = pi./(4*northrings(cap));
    
    % Handle rest
    rest = ~cap;
    theta(rest) = acos((2*nside - northrings(rest)) * (8*nside/npix));
    nphi(rest) = 4*nside;
    phi0(rest) = (pi/(4*nside)) * double(mod(northrings(rest) - nside, 2) == 0);
    
    % Above assumed northern hemisphere. Fix southern
    south = northrings ~= rings;
    theta(south) = pi - theta(south);
    
    weight = 4*pi/npix;
    
    % Compute ringstart (offsets)
    off = [0; cumsum(double(nphi(1:end-1)))];
    ringstart = uint64(off);
    
    sht_info = struct();
    sht_info.nring = nring;
    sht_info.theta = theta;
    sht_info.phi0 = phi0;
    sht_info.nphi = nphi;
    sht_info.weight = weight;
    sht_info.ringstart = ringstart;
    sht_info.nside = nside;
end

