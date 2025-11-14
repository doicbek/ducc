function n = good_size(n, varargin)
%GOOD_SIZE Returns a "good" size for FFT operations
%   N = GOOD_SIZE(N) returns a good size >= N for complex FFTs.
%   N = GOOD_SIZE(N, 'real', true) returns a good size for real FFTs.
%
%   This function returns a size that is efficient for FFT operations,
%   typically a number with small prime factors (2, 3, 5, 7, 11).
%
%   Parameters
%   ----------
%   n : int
%       Target length (must be positive).
%   real : bool, optional
%       If true, find good size for real FFT (default: false).
%
%   Returns
%   -------
%   n : int
%       Efficient FFT size >= input n.
%
%   Example
%   -------
%   n = 1000;
%   n_good = ducc0.fft.good_size(n);  % Returns 1008 (or similar)
%   n_good_real = ducc0.fft.good_size(n, 'real', true);

    p = inputParser;
    addRequired(p, 'n', @(x) isnumeric(x) && isscalar(x) && x > 0);
    addParameter(p, 'real', false, @(x) islogical(x) || isnumeric(x));
    parse(p, n, varargin{:});
    
    % Convert real to logical
    real_flag = logical(p.Results.real);
    
    % Call MEX function
    n = ducc0_fft_good_size_mex(double(p.Results.n), real_flag);
end
