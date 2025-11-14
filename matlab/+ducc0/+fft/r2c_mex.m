function out = r2c_mex(a, varargin)
%R2C_MEX Performs a real-to-complex FFT using MEX interface
%   This is the MEX-based version of r2c, providing direct C++ access.
%
%   See ducc0.fft.r2c for full documentation.

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @islogical);
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    % Ensure double precision
    a_double = double(a);
    
    % Prepare axes
    if isempty(p.Results.axes)
        axes = [];
    else
        axes = double(p.Results.axes(:)');
    end
    
    % Call MEX function
    out = ducc0_fft_r2c_mex(a_double, axes, p.Results.forward, ...
        p.Results.inorm, p.Results.nthreads);
end

