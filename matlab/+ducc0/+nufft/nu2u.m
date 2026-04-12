function out = nu2u(points, coord, varargin)
%NU2U Non-uniform to uniform FFT (type 1: points to grid)
%   OUT = NU2U(POINTS, COORD) transforms non-uniform point values to a uniform grid.
%   OUT = NU2U(POINTS, COORD, 'grid_shape', SHAPE) sets output grid shape.
%   OUT = NU2U(POINTS, COORD, 'forward', true) uses forward transform (default: true).
%   OUT = NU2U(POINTS, COORD, 'epsilon', EPS) sets accuracy (default: 1e-10).
%   OUT = NU2U(POINTS, COORD, 'periodicity', PER) sets periodicity.
%   OUT = NU2U(POINTS, COORD, 'fft_order', true) uses FFT ordering (default: true).
%   OUT = NU2U(POINTS, COORD, 'nthreads', N) uses N threads.
%
%   Parameters
%   ----------
%   points : complex array
%       Non-uniform point values, shape [npoints] or [ncomp, npoints].
%   coord : numeric array
%       Coordinates of points, shape [npoints, ndim].
%   grid_shape : int array, optional
%       Shape of output grid (default: 64 per dimension).
%   forward : bool, optional
%       Forward transform direction (default: true).
%   epsilon : double, optional
%       Target accuracy (default: 1e-10).
%   periodicity : double or double array, optional
%       Periodicity in each dimension (default: 2*pi).
%   fft_order : bool, optional
%       Use FFT ordering (default: true).
%   verbosity : int, optional
%       Verbosity level (default: 0).
%   sigma_min : double, optional
%       Minimum oversampling factor (default: 1.2).
%   sigma_max : double, optional
%       Maximum oversampling factor (default: 2.5).
%   nthreads : int, optional
%       Number of threads (default: 0 = system default).
%
%   Returns
%   -------
%   out : complex array
%       Uniform grid values, shape [grid_shape] or [ncomp, grid_shape].
%
%   See also: ducc0.nufft.u2nu

    p = inputParser;
    addRequired(p, 'points', @(x) isnumeric(x));
    addRequired(p, 'coord', @(x) isnumeric(x) && isreal(x));
    addParameter(p, 'grid_shape', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'forward', true, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'epsilon', 1e-10, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'periodicity', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'fft_order', true, @(x) islogical(x) || isnumeric(x));
    addParameter(p, 'verbosity', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'sigma_min', 1.2, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'sigma_max', 2.5, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, points, coord, varargin{:});

    if isreal(points)
        points = complex(points);
    end

    grid_shape = [];
    if ~isempty(p.Results.grid_shape)
        grid_shape = double(p.Results.grid_shape(:)');
    end

    periodicity = [];
    if ~isempty(p.Results.periodicity)
        periodicity = double(p.Results.periodicity(:));
    end

    out = ducc0_nufft_nu2u_mex(points, coord, logical(p.Results.forward), ...
        p.Results.epsilon, double(p.Results.nthreads), grid_shape, ...
        double(p.Results.verbosity), p.Results.sigma_min, p.Results.sigma_max, ...
        periodicity, logical(p.Results.fft_order));
end
