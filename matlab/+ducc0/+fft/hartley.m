function out = hartley(a, varargin)
%HARTLEY Performs a separable Hartley transform
%   OUT = HARTLEY(A) performs a separable Hartley transform on all axes of A.
%   OUT = HARTLEY(A, 'axes', AXES) transforms only the specified axes (1-based).
%   OUT = HARTLEY(A, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = HARTLEY(A, 'nthreads', N) uses N threads (0 = system default).
%
%   Parameters
%   ----------
%   a : numeric array (real)
%       The input data (must be real).
%   axes : int array, optional
%       The axes along which the Hartley transform is carried out (1-based, MATLAB convention).
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
    addParameter(p, 'inorm', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, a, varargin{:});
    
    % Prepare axes (1-based MATLAB convention)
    axes = [];
    if ~isempty(p.Results.axes)
        axes = double(p.Results.axes(:)');  % Row vector, 1-based
    end
    
    % Call MEX function
    out = ducc0_fft_hartley_mex(a, axes, p.Results.inorm, p.Results.nthreads);
end

