function weights = get_gridweights(geometry, ntheta)
%GET_GRIDWEIGHTS Returns quadrature weights for a given grid geometry
%   WEIGHTS = GET_GRIDWEIGHTS(GEOMETRY, NTHETA) returns quadrature weights.
%
%   Parameters
%   ----------
%   geometry : string
%       Grid geometry: 'CC', 'F1', 'MW', 'MWflip', 'GL', 'DH', 'F2'
%   ntheta : int
%       Number of rings in the grid
%
%   Returns
%   -------
%   weights : double array
%       Quadrature weights for individual rings (size [ntheta])
%
%   Note: These weights need to be divided by the number of pixels per ring
%   to obtain actual quadrature weights for a particular map.
%
%   Example
%   -------
%   weights = ducc0.sht.get_gridweights('CC', 64);
%   weights_per_pixel = weights ./ nphi;  % Divide by pixels per ring

    % Validate inputs
    if ~ischar(geometry) && ~isstring(geometry)
        error('DUCC0:SHT:GetGridweights:InputError', 'geometry must be a string');
    end
    if ~isnumeric(ntheta) || ~isscalar(ntheta) || ntheta <= 0
        error('DUCC0:SHT:GetGridweights:InputError', 'ntheta must be a positive integer');
    end
    
    % Convert string to char if needed
    geometry = char(geometry);
    
    % Call MEX function
    weights = ducc0_sht_get_gridweights_mex(geometry, double(ntheta));
end
