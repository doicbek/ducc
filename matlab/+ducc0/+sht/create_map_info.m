function map_info = create_map_info(nside)
%CREATE_MAP_INFO Create map info structure for HEALPix
%   MAP_INFO = CREATE_MAP_INFO(NSIDE) creates map info for HEALPix with given nside.
%
%   Parameters
%   ----------
%   nside : int
%       HEALPix nside parameter (resolution parameter)
%
%   Returns
%   -------
%   map_info : struct
%       Map information structure containing pixelization details
%
%   See also: create_alm_info, create_sht_info

    if ~isscalar(nside) || ~isnumeric(nside) || nside < 1
        error('DUCC0:SHT:CreateMapInfo:InputError', ...
            'nside must be a positive scalar integer');
    end
    
    nside = double(nside);
    npix = 12*nside*nside;
    
    map_info = struct();
    map_info.is_healpix = true;
    map_info.nside = nside;
    map_info.npix = npix;
    
    % Create SHT info and store it
    map_info.si = ducc0.sht.create_sht_info(map_info);
end

