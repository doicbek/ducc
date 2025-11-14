function out = u2nu(grid, coord, varargin)
%U2NU Uniform to non-uniform FFT (grid to points)
%   OUT = U2NU(GRID, COORD) transforms uniform grid to non-uniform points.
%   OUT = U2NU(GRID, COORD, 'forward', false) uses inverse transform (default: false).
%   OUT = U2NU(GRID, COORD, 'epsilon', EPS) sets accuracy (default: 1e-12).
%   OUT = U2NU(GRID, COORD, 'periodicity', PER) sets periodicity.
%   OUT = U2NU(GRID, COORD, 'fft_order', true) uses FFT ordering.
%   OUT = U2NU(GRID, COORD, 'nthreads', N) uses N threads.
%
%   This is the inverse operation of nu2u.
%
%   See also: ducc0.nufft.nu2u, ducc0.nufft.nu2nu

    p = inputParser;
    addRequired(p, 'grid', @(x) isnumeric(x));
    addRequired(p, 'coord', @(x) isnumeric(x));
    addParameter(p, 'forward', false, @islogical);
    addParameter(p, 'epsilon', 1e-12, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'periodicity', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'fft_order', false, @islogical);
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'out', [], @(x) isnumeric(x) || isempty(x));
    parse(p, grid, coord, varargin{:});
    
    py_grid = ducc0.util.matlab2numpy(p.Results.grid);
    py_coord = ducc0.util.matlab2numpy(p.Results.coord);
    
    kwargs = py.dict();
    kwargs{'forward'} = p.Results.forward;
    kwargs{'epsilon'} = double(p.Results.epsilon);
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
    py_result = py_mod.nufft.u2nu(py_grid, py_coord, pyargs(kwargs));
    out = ducc0.util.numpy2matlab(py_result);
end

