function out = r2c(a, varargin)
%R2C Performs an FFT whose input is strictly real
%   OUT = R2C(A) performs a real-to-complex FFT on all axes of A.
%   OUT = R2C(A, 'axes', AXES) transforms only the specified axes (1-based).
%   OUT = R2C(A, 'forward', true) uses negative sign in exponent (default: true).
%   OUT = R2C(A, 'inorm', INORM) sets normalization:
%        0: no normalization
%        1: divide by sqrt(N)
%        2: divide by N
%   OUT = R2C(A, 'nthreads', N) uses N threads (0 = system default).
%
%   The output shape is identical to input except for the last transformed axis,
%   which is reduced from n to n//2+1.
%
%   Parameters
%   ----------
%   a : numeric array (real)
%       The input data (must be real).
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
%       The transformed data. Shape is identical to input except for last axis.
%
%   See also: ducc0.fft.c2c, ducc0.fft.c2r

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    % Prepare axes (1-based MATLAB convention)
    axes = [];
    if ~isempty(p.Results.axes)
        axes = double(p.Results.axes(:)');  % Row vector, 1-based
    end
    
    % Call MEX function
    out = ducc0_fft_r2c_mex(a, axes, logical(p.Results.forward), ...
        p.Results.inorm, p.Results.nthreads);
end
