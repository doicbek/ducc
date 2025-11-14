function out = nu2u(points, coord, varargin)
%NU2U Non-uniform to uniform FFT (points to grid)
%   OUT = NU2U(POINTS, COORD) transforms non-uniform points to uniform grid.
%   OUT = NU2U(POINTS, COORD, 'forward', true) uses forward transform (default: true).
%   OUT = NU2U(POINTS, COORD, 'epsilon', EPS) sets accuracy (default: 1e-12).
%   OUT = NU2U(POINTS, COORD, 'grid_shape', SHAPE) sets output grid shape.
%   OUT = NU2U(POINTS, COORD, 'periodicity', PER) sets periodicity.
%   OUT = NU2U(POINTS, COORD, 'fft_order', true) uses FFT ordering.
%   OUT = NU2U(POINTS, COORD, 'nthreads', N) uses N threads.
%
%   Parameters
%   ----------
%   points : complex array
%       Non-uniform point values, shape (npoints,) or (ntrans, npoints)
%   coord : numeric array
%       Coordinates of points, shape (npoints, ndim)
%   forward : bool, optional
%       Forward transform direction (default: true)
%   epsilon : double, optional
%       Target accuracy (default: 1e-12)
%   grid_shape : int array, optional
%       Shape of output grid (required if out not provided)
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
%       Uniform grid values
%
%   See also: ducc0.nufft.u2nu, ducc0.nufft.nu2nu

    p = inputParser;
    addRequired(p, 'points', @(x) isnumeric(x));
    addRequired(p, 'coord', @(x) isnumeric(x));
    addParameter(p, 'forward', true, @islogical);
    addParameter(p, 'epsilon', 1e-12, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'grid_shape', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'periodicity', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'fft_order', false, @islogical);
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'out', [], @(x) isnumeric(x) || isempty(x));
    parse(p, points, coord, varargin{:});
    
    py_points = ducc0.util.matlab2numpy(p.Results.points);
    py_coord = ducc0.util.matlab2numpy(p.Results.coord);
    
    kwargs = py.dict();
    kwargs{'forward'} = p.Results.forward;
    kwargs{'epsilon'} = double(p.Results.epsilon);
    if ~isempty(p.Results.grid_shape)
        kwargs{'grid_shape'} = py.list(int32(p.Results.grid_shape));
    end
    if ~isempty(p.Results.periodicity)
        if isscalar(p.Results.periodicity)
            kwargs{'periodicity'} = double(p.Results.periodicity);
        else
            kwargs{'periodicity'} = py.list(double(p.Results.periodicity));
        end
    end
    kwargs{'fft_order'} = p.Results.fft_order;
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    if ~isempty(p.Results.out)
        kwargs{'out'} = ducc0.util.matlab2numpy(p.Results.out);
    end
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.nufft.nu2u(py_points, py_coord, pyargs(kwargs));
    out = ducc0.util.numpy2matlab(py_result);
end

