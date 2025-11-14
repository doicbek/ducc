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
%       Uniform grid values
%   coord : numeric array
%       Coordinates of points, shape (npoints, ndim)
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
%       Non-uniform point values

    error('DUCC0:MEX:NotImplemented', ...
        'u2nu MEX function is not yet implemented.');

    % TODO: Implement MEX function
end
