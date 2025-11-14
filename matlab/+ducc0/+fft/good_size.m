function n = good_size(n, varargin)
%GOOD_SIZE Returns a "good" size for FFT operations
%   N = GOOD_SIZE(N) returns a good size >= N for complex FFTs.
%   N = GOOD_SIZE(N, 'real', true) returns a good size for real FFTs.
%
%   This function returns a size that is efficient for FFT operations,
%   typically a number with small prime factors.
%
%   Example
%   -------
%   n = 1000;
%   n_good = ducc0.fft.good_size(n);  % Returns 1024

    p = inputParser;
    addRequired(p, 'n', @(x) isnumeric(x) && isscalar(x) && x > 0);
    addParameter(p, 'real', false, @islogical);
    parse(p, n, varargin{:});
    
    py_mod = ducc0.ducc0();
    if p.Results.real
        py_result = py_mod.fft.good_size(int32(p.Results.n), pyargs('real', true));
    else
        py_result = py_mod.fft.good_size(int32(p.Results.n));
    end
    n = double(py_result);
end

