function out = c2c_mex(a, varargin)
%C2C_MEX Performs a complex FFT using MEX interface
%   This is the MEX-based version of c2c, providing direct C++ access.
%
%   See ducc0.fft.c2c for full documentation.

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @islogical);
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    % Ensure input is in correct format
    if ~isreal(a)
        a_complex = complex(double(real(a)), double(imag(a)));
    else
        % Real input - convert to complex
        a_complex = complex(double(a), zeros(size(a)));
    end
    
    % Prepare axes (convert from MATLAB 1-based to 0-based, reversed)
    if isempty(p.Results.axes)
        axes = [];
    else
        axes = double(p.Results.axes(:)');  % Row vector
    end
    
    % Call MEX function
    out = ducc0_fft_mex(a_complex, axes, p.Results.forward, ...
        p.Results.inorm, p.Results.nthreads);
end

