function alm_info = create_alm_info(lmax)
%CREATE_ALM_INFO Create alm info structure
%   ALM_INFO = CREATE_ALM_INFO(LMAX) creates alm information structure.
%
%   Parameters
%   ----------
%   lmax : int
%       Maximum multipole order
%
%   Returns
%   -------
%   alm_info : struct
%       Alm information structure containing:
%       - lmax: Maximum multipole order
%       - mmax: Maximum m order (same as lmax)
%       - mstart: Starting index for each m value, size [mmax+1]
%       - nelem: Total number of alm coefficients
%
%   See also: create_map_info, create_sht_info

    if ~isscalar(lmax) || ~isnumeric(lmax) || lmax < 0
        error('DUCC0:SHT:CreateAlmInfo:InputError', ...
            'lmax must be a non-negative scalar');
    end
    
    lmax = double(lmax);
    mmax = lmax;
    m = (0:mmax)';
    mstart = uint64(m .* (2*lmax + 1 - m) / 2);
    nelem = uint64(max(mstart) + lmax + 1);
    
    alm_info = struct();
    alm_info.lmax = lmax;
    alm_info.mmax = mmax;
    alm_info.mstart = mstart;
    alm_info.nelem = nelem;
end

