function maps_unpad = unpad_map(maps, sht_info)
%UNPAD_MAP Unpad map (no-op for HEALPix)
%   MAPS_UNPAD = UNPAD_MAP(MAPS, SHT_INFO) returns maps unchanged for HEALPix.
%
%   Parameters
%   ----------
%   maps : numeric array
%       Input maps, shape [nmaps, npix] or [npix]
%   sht_info : struct
%       SHT info structure from create_sht_info
%
%   Returns
%   -------
%   maps_unpad : numeric array
%       Maps unchanged (no unpadding needed for HEALPix)
%
%   See also: pad_map

    % No unpadding needed for HEALPix
    maps_unpad = maps;
end

