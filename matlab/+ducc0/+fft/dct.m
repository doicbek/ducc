function out = dct(a, type, varargin)
%DCT Performs a discrete cosine transform
%   OUT = DCT(A, TYPE) performs a discrete cosine transform of type TYPE.
%   OUT = DCT(A, TYPE, 'axes', AXES) transforms only the specified axes (1-based).
%   OUT = DCT(A, TYPE, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = DCT(A, TYPE, 'nthreads', N) uses N threads (0 = system default).
%
%   Parameters
%   ----------
%   a : numeric array (real)
%       The input data (must be real).
%   type : int
%       DCT type: 1, 2, 3, or 4.
%   axes : int array, optional
%       The axes along which the DCT is carried out (1-based, MATLAB convention).
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
%   See also: ducc0.fft.dst, ducc0.fft.c2c

    p = inputParser;
    addRequired(p, 'a', @(x) isnumeric(x) && isreal(x));
    addRequired(p, 'type', @(x) isnumeric(x) && isscalar(x) && x >= 1 && x <= 4);
    addParameter(p, 'axes', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, type, varargin{:});
    
    % Prepare axes (1-based MATLAB convention)
    axes = [];
    if ~isempty(p.Results.axes)
        axes = double(p.Results.axes(:)');  % Row vector, 1-based
    end
    
    % Call MEX function
    out = ducc0_fft_dct_mex(a, p.Results.type, axes, p.Results.inorm, p.Results.nthreads);
end

