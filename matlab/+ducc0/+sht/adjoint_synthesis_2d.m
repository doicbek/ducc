function map = adjoint_synthesis_2d(alm, lmax, varargin)
%ADJOINT_SYNTHESIS_2D Performs adjoint spherical harmonic synthesis
%   MAP = ADJOINT_SYNTHESIS_2D(ALM, LMAX) performs adjoint synthesis.
%   This is the adjoint operation of analysis_2d.
%
%   See ducc0.sht.synthesis_2d for parameter details.
%
%   See also: ducc0.sht.analysis_2d, ducc0.sht.adjoint_analysis_2d

    p = inputParser;
    addRequired(p, 'alm', @(x) isnumeric(x));
    addRequired(p, 'lmax', @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'mmax', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'spin', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'ntheta', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'nphi', [], @(x) isnumeric(x) || isempty(x));
    addParameter(p, 'geometry', 'CC', @(x) ischar(x) || isstring(x));
    addParameter(p, 'phi0', 0, @(x) isnumeric(x) && isscalar(x));
    addParameter(p, 'nthreads', 0, @(x) isnumeric(x) && isscalar(x));
    parse(p, alm, lmax, varargin{:});
    
    py_alm = ducc0.util.matlab2numpy(p.Results.alm);
    kwargs = py.dict();
    kwargs{'lmax'} = int32(p.Results.lmax);
    if ~isempty(p.Results.mmax)
        kwargs{'mmax'} = int32(p.Results.mmax);
    end
    kwargs{'spin'} = int32(p.Results.spin);
    if ~isempty(p.Results.ntheta)
        kwargs{'ntheta'} = int32(p.Results.ntheta);
    end
    if ~isempty(p.Results.nphi)
        kwargs{'nphi'} = int32(p.Results.nphi);
    end
    kwargs{'geometry'} = char(p.Results.geometry);
    kwargs{'phi0'} = double(p.Results.phi0);
    kwargs{'nthreads'} = int32(p.Results.nthreads);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.sht.adjoint_synthesis_2d(py_alm, pyargs(kwargs));
    map = ducc0.util.numpy2matlab(py_result);
end

