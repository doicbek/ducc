function out = r2r_fftpack(a, varargin)
%R2R_FFTPACK Performs a real-to-real FFT using FFTPACK conventions
%   OUT = R2R_FFTPACK(A) performs a real-to-real FFT.
%   OUT = R2R_FFTPACK(A, 'axes', AXES) transforms only the specified axes (1-based).
%   OUT = R2R_FFTPACK(A, 'real2hermitian', true) uses real-to-Hermitian (default: true).
%   OUT = R2R_FFTPACK(A, 'forward', true) uses forward transform (default: true).
%   OUT = R2R_FFTPACK(A, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = R2R_FFTPACK(A, 'nthreads', N) uses N threads (0 = system default).
%
%   Parameters
%   ----------
%   a : numeric array (real)
%       The input data (must be real).
%   axes : int array, optional
%       The axes along which the FFT is carried out (1-based, MATLAB convention).
%   real2hermitian : bool, optional
%       If true, uses real-to-Hermitian transform (default: true).
%   forward : bool, optional
%       If true, uses forward transform (default: true).
%   inorm : int, optional
%       Normalization type: 0 (none), 1 (1/sqrt(N)), 2 (1/N) (default: 0).
%   nthreads : int, optional
%       Number of threads to use. 0 uses system default (default: 0).
%
%   Returns
%   -------
%   out : numeric array (real)
%       The transformed data (same shape as input).
%
%   See also: ducc0.fft.c2c, ducc0.fft.r2c

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'real2hermitian', true, @(x) islogical(x) || isnumeric(x));
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
    out = ducc0_fft_r2r_fftpack_mex(a, axes, logical(p.Results.real2hermitian), ...
        logical(p.Results.forward), p.Results.inorm, p.Results.nthreads);
end
