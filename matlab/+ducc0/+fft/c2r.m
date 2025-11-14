function out = c2r(a, varargin)
%C2R Performs an FFT whose output is strictly real
%   OUT = C2R(A) performs a complex-to-real FFT on all axes of A.
%   OUT = C2R(A, 'axes', AXES) transforms only the specified axes.
%   OUT = C2R(A, 'lastsize', N) sets output size of last axis (default: 2*n-2).
%   OUT = C2R(A, 'forward', false) uses positive sign (default: false for inverse).
%   OUT = C2R(A, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = C2R(A, 'out', OUT) uses pre-allocated output array.
%   OUT = C2R(A, 'nthreads', N) uses N threads (0 = system default).
%
%   See also: ducc0.fft.c2c, ducc0.fft.r2c

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'lastsize', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'forward', false, @islogical);
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'out', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    py_a = ducc0.util.matlab2numpy(p.Results.a);
    kwargs = py.dict();
    if ~isempty(p.Results.axes)
        kwargs{'axes'} = py.list(int32(p.Results.axes));
    end
    if p.Results.lastsize > 0
        kwargs{'lastsize'} = int32(p.Results.lastsize);
    end
    kwargs{'forward'} = p.Results.forward;
    kwargs{'inorm'} = int32(p.Results.inorm);
    if ~isempty(p.Results.out)
        kwargs{'out'} = ducc0.util.matlab2numpy(p.Results.out);
    end
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.fft.c2r(py_a, pyargs(kwargs));
    out = ducc0.util.numpy2matlab(py_result);
end

