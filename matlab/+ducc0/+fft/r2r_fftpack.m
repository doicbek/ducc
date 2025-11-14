function out = r2r_fftpack(a, varargin)
%R2R_FFTPACK Performs a real-valued FFT using FFTPACK's halfcomplex storage
%   OUT = R2R_FFTPACK(A) performs a real-to-real FFT using FFTPACK convention.
%   OUT = R2R_FFTPACK(A, 'axes', AXES) transforms only specified axes.
%   OUT = R2R_FFTPACK(A, 'real2hermitian', true) input is real, output has Hermitian symmetry.
%   OUT = R2R_FFTPACK(A, 'forward', true) uses negative sign (default: true).
%   OUT = R2R_FFTPACK(A, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = R2R_FFTPACK(A, 'out', OUT) uses pre-allocated output array.
%   OUT = R2R_FFTPACK(A, 'nthreads', N) uses N threads (0 = system default).
%
%   See also: ducc0.fft.r2r_fftw, ducc0.fft.c2c

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'real2hermitian', true, @islogical);
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
    kwargs{'real2hermitian'} = p.Results.real2hermitian;
    kwargs{'forward'} = p.Results.forward;
    kwargs{'inorm'} = int32(p.Results.inorm);
    if ~isempty(p.Results.out)
        kwargs{'out'} = ducc0.util.matlab2numpy(p.Results.out);
    end
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.fft.r2r_fftpack(py_a, pyargs(kwargs));
    out = ducc0.util.numpy2matlab(py_result);
end

