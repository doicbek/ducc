function alm = analysis_2d(map, lmax, varargin)
%ANALYSIS_2D Performs spherical harmonic analysis (map2alm) for 2D grids
%   ALM = ANALYSIS_2D(MAP, LMAX) analyzes a map into spherical harmonic coefficients.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'mmax', MMAX) sets maximum m order.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'spin', SPIN) sets spin value (0, 1, or 2).
%   ALM = ANALYSIS_2D(MAP, LMAX, 'geometry', GEO) sets grid geometry.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'phi0', PHI0) sets phi offset.
%   ALM = ANALYSIS_2D(MAP, LMAX, 'nthreads', N) uses N threads.
%
%   This is the inverse operation of synthesis_2d.
%
%   See also: ducc0.sht.synthesis_2d, ducc0.sht.adjoint_analysis_2d

    error('DUCC0:MEX:NotImplemented', ...
        'analysis_2d MEX function is not yet implemented.');

    % TODO: Implement MEX function
end
