function out = c2c(a, varargin)
%C2C Performs a complex FFT
%   OUT = C2C(A) performs a complex FFT on all axes of A.
%   OUT = C2C(A, 'axes', AXES) transforms only the specified axes (1-based).
%   OUT = C2C(A, 'forward', true) uses negative sign in exponent (default: true).
%   OUT = C2C(A, 'inorm', INORM) sets normalization:
%        0: no normalization
%        1: divide by sqrt(N)
%        2: divide by N
%   OUT = C2C(A, 'nthreads', N) uses N threads (0 = system default).
%
%   Parameters
%   ----------
%   a : numeric array (real or complex)
%       The input data. If real, a more efficient real-to-complex transform is used.
%   axes : int array, optional
%       The axes along which the FFT is carried out (1-based, MATLAB convention).
%       If not set, all axes will be transformed.
%   forward : bool, optional
%       If true, a negative sign is used in the exponent (default: true).
%   inorm : int, optional
%       Normalization type: 0 (none), 1 (1/sqrt(N)), 2 (1/N) (default: 0).
%   nthreads : int, optional
%       Number of threads to use. 0 uses system default (default: 0).
%
%   Returns
%   -------
%   out : numeric array (complex)
%       The transformed data, same shape as input (or complex type).
%
%   Example
%   -------
%   x = randn(128, 128) + 1i*randn(128, 128);
%   y = ducc0.fft.c2c(x);
%   z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);  % Inverse transform
%
%   See also: ducc0.fft.r2c, ducc0.fft.c2r

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    % Ensure input is numeric
    if ~isnumeric(a)
        error('DUCC0:FFT:C2C:InputError', 'Input must be numeric');
    end
    
    % Convert forward to logical
    forward = logical(p.Results.forward);
    
    % Prepare axes (1-based MATLAB convention)
    axes = [];
    if ~isempty(p.Results.axes)
        axes = double(p.Results.axes(:)');  % Row vector, 1-based
    end
    
    % Call MEX function
    % MEX function expects: (a, axes, forward, inorm, nthreads)
    out = ducc0_fft_c2c_mex(a, axes, forward, p.Results.inorm, p.Results.nthreads);
end
