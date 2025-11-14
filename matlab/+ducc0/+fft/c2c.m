function out = c2c(a, varargin)
%C2C Performs a complex FFT
%   OUT = C2C(A) performs a complex FFT on all axes of A.
%   OUT = C2C(A, 'axes', AXES) transforms only the specified axes.
%   OUT = C2C(A, 'forward', true) uses negative sign in exponent (default: true).
%   OUT = C2C(A, 'inorm', INORM) sets normalization:
%        0: no normalization
%        1: divide by sqrt(N)
%        2: divide by N
%   OUT = C2C(A, 'out', OUT) uses pre-allocated output array.
%   OUT = C2C(A, 'nthreads', N) uses N threads (0 = system default).
%
%   Parameters
%   ----------
%   a : numeric array (real or complex)
%       The input data. If real, a more efficient real-to-complex transform is used.
%   axes : int array, optional
%       The axes along which the FFT is carried out (0-indexed).
%       If not set, all axes will be transformed.
%   forward : bool, optional
%       If true, a negative sign is used in the exponent (default: true).
%   inorm : int, optional
%       Normalization type: 0 (none), 1 (1/sqrt(N)), 2 (1/N) (default: 0).
%   out : numeric array, optional
%       Pre-allocated output array. Must not overlap with input.
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
%   z = ducc0.fft.c2c(y, 'forward', false);  % Inverse transform
%
%   See also: ducc0.fft.r2c, ducc0.fft.c2r

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @islogical);
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'out', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    % Convert MATLAB array to Python numpy array
    py_a = ducc0.util.matlab2numpy(p.Results.a);
    
    % Prepare keyword arguments
    kwargs = py.dict();
    if ~isempty(p.Results.axes)
        kwargs{'axes'} = py.list(int32(p.Results.axes));
    end
    kwargs{'forward'} = p.Results.forward;
    kwargs{'inorm'} = int32(p.Results.inorm);
    if ~isempty(p.Results.out)
        kwargs{'out'} = ducc0.util.matlab2numpy(p.Results.out);
    end
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    
    % Call Python function
    py_mod = ducc0.ducc0();
    py_result = py_mod.fft.c2c(py_a, pyargs(kwargs));
    
    % Convert back to MATLAB
    out = ducc0.util.numpy2matlab(py_result);
end

