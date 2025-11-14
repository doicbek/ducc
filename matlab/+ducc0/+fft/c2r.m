function out = c2r(a, varargin)
%C2R Performs an FFT whose output is strictly real
%   OUT = C2R(A) performs a complex-to-real FFT on all axes of A.
%   OUT = C2R(A, 'axes', AXES) transforms only the specified axes (1-based).
%   OUT = C2R(A, 'lastsize', N) sets output size of last axis (default: 2*n-2).
%   OUT = C2R(A, 'forward', false) uses positive sign (default: false for inverse).
%   OUT = C2R(A, 'inorm', INORM) sets normalization (0/1/2).
%   OUT = C2R(A, 'nthreads', N) uses N threads (0 = system default).
%
%   Parameters
%   ----------
%   a : numeric array (complex)
%       The input data (must be complex).
%   axes : int array, optional
%       The axes along which the FFT is carried out (1-based, MATLAB convention).
%       If not set, all axes will be transformed.
%   lastsize : int, optional
%       Output size of last axis (default: 2*n-2, where n is input size).
%   forward : bool, optional
%       If true, a negative sign is used in the exponent (default: false).
%   inorm : int, optional
%       Normalization type: 0 (none), 1 (1/sqrt(N)), 2 (1/N) (default: 0).
%   nthreads : int, optional
%       Number of threads to use. 0 uses system default (default: 0).
%
%   Returns
%   -------
%   out : numeric array (real)
%       The transformed data (real output).
%
%   See also: ducc0.fft.c2c, ducc0.fft.r2c

    error('DUCC0:MEX:NotImplemented', ...
        'c2r MEX function is not yet implemented.');

    % TODO: Implement MEX function
end
