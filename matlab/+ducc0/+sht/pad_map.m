function maps_pad = pad_map(maps, sht_info)
%PAD_MAP Pad map (no-op for HEALPix)
%   MAPS_PAD = PAD_MAP(MAPS, SHT_INFO) returns maps unchanged for HEALPix.
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
%   maps_pad : numeric array
%       Maps unchanged (no padding needed for HEALPix)
%
%   See also: unpad_map

    % No padding needed for HEALPix
    maps_pad = maps;
end

