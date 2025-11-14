function alm = adjoint_analysis_2d(map, lmax, varargin)
%ADJOINT_ANALYSIS_2D Performs adjoint spherical harmonic analysis
%   ALM = ADJOINT_ANALYSIS_2D(MAP, LMAX) performs adjoint analysis.
%   This is the adjoint operation of synthesis_2d.
%
%   See ducc0.sht.analysis_2d for parameter details.
%
%   See also: ducc0.sht.synthesis_2d, ducc0.sht.adjoint_synthesis_2d

    p = inputParser;
    addRequired(p, 'map', @(x) isnumeric(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, map, lmax, varargin{:});
    
    py_map = ducc0.util.matlab2numpy(p.Results.map);
    kwargs = py.dict();
    kwargs{'lmax'} = int32(p.Results.lmax);
    if ~isempty(p.Results.mmax)
        kwargs{'mmax'} = int32(p.Results.mmax);
    end
    kwargs{'spin'} = int32(p.Results.spin);
    kwargs{'geometry'} = char(p.Results.geometry);
    kwargs{'phi0'} = double(p.Results.phi0);
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.sht.adjoint_analysis_2d(py_map, pyargs(kwargs));
    alm = ducc0.util.numpy2matlab(py_result);
end

