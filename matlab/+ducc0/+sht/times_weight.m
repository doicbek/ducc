function m_weighted = times_weight(m, sht_info)
%TIMES_WEIGHT Multiply map by quadrature weights
%   M_WEIGHTED = TIMES_WEIGHT(M, SHT_INFO) multiplies map by quadrature weights.
%
%   Parameters
%   ----------
%   m : numeric array
%       Input map, shape [nmaps, npix] or [npix]
%   sht_info : struct
%       SHT info structure from create_sht_info
%
%   Returns
%   -------
%   m_weighted : numeric array
%       Weighted map (same shape as input)
%
%   See also: pad_map, unpad_map

    % Weight is constant for HEALPix
    m_weighted = m * sht_info.weight;
end

