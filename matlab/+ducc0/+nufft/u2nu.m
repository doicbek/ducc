function out = u2nu(grid, coord, varargin)
%U2NU Uniform to non-uniform FFT (grid to points)
%   OUT = U2NU(GRID, COORD) transforms uniform grid to non-uniform points.
%   OUT = U2NU(GRID, COORD, 'forward', true) uses forward transform (default: true).
%   OUT = U2NU(GRID, COORD, 'epsilon', EPS) sets accuracy (default: 1e-12).
%   OUT = U2NU(GRID, COORD, 'periodicity', PER) sets periodicity.
%   OUT = U2NU(GRID, COORD, 'fft_order', true) uses FFT ordering.
%   OUT = U2NU(GRID, COORD, 'nthreads', N) uses N threads.
%
%   Parameters
%   ----------
%   grid : complex array
%       Uniform grid values, shape [gridshape] or [ncomp, gridshape]
%   coord : numeric array
%       Coordinates of points, shape [npoints, ndim]
%   forward : bool, optional
%       Forward transform direction (default: true)
%   epsilon : double, optional
%       Target accuracy (default: 1e-12)
%   periodicity : double or double array, optional
%       Periodicity in each dimension (default: 2*pi)
%   fft_order : bool, optional
%       Use FFT ordering (default: false)
%   nthreads : int, optional
%       Number of threads (default: 0)
%
%   Returns
%   -------
%   out : complex array
%       Non-uniform point values, shape [npoints] or [ncomp, npoints]

    p = inputParser;
    addRequired(p, 'grid', @(x) isnumeric(x) && ~isreal(x));
    addRequired(p, 'coord', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'forward', true, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'epsilon', 1e-12, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'periodicity', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'fft_order', false, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, grid, coord, varargin{:});
    
    periodicity = []; if ~isempty(p.Results.periodicity), periodicity = double(p.Results.periodicity(:)); end
    
    out = ducc0_nufft_u2nu_mex(grid, coord, logical(p.Results.forward), ...
        p.Results.epsilon, periodicity, logical(p.Results.fft_order), ...
        p.Results.nthreads);
end
