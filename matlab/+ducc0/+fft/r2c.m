function out = r2c(a, varargin)
%R2C Performs an FFT whose input is strictly real
%   OUT = R2C(A) performs a real-to-complex FFT on all axes of A.
%   OUT = R2C(A, 'axes', AXES) transforms only the specified axes.
%   OUT = R2C(A, 'forward', true) uses negative sign in exponent (default: true).
%   OUT = R2C(A, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = R2C(A, 'out', OUT) uses pre-allocated output array.
%   OUT = R2C(A, 'nthreads', N) uses N threads (0 = system default).
%
%   The output shape is identical to input except for the last transformed axis,
%   which is reduced from n to n//2+1.
%
%   See also: ducc0.fft.c2c, ducc0.fft.c2r

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @islogical);
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'out', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    py_a = ducc0.util.matlab2numpy(p.Results.a);
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
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.fft.r2c(py_a, pyargs(kwargs));
    out = ducc0.util.numpy2matlab(py_result);
end

