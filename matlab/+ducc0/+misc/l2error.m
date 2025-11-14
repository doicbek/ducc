function err = l2error(a, b)
%L2ERROR Compute L2 error between two arrays
%   ERR = L2ERROR(A, B) computes the L2 error between arrays A and B.
%
%   The L2 error is defined as sqrt(sum(|a_i - b_i|^2)).
%
%   Parameters
%   ----------
%   a : numeric array
%       First array
%   b : numeric array
%       Second array, same shape as a
%
%   Returns
%   -------
%   err : double
%       L2 error between the arrays

    % Simple MATLAB implementation (no MEX needed for this simple function)
    if nargin < 2
        error('DUCC0:InvalidInput', 'Both a and b are required');
    end
    
    if ~isnumeric(a) || ~isnumeric(b)
        error('DUCC0:InvalidInput', 'Inputs must be numeric arrays');
    end
    
    if ~isequal(size(a), size(b))
        error('DUCC0:InvalidInput', 'Inputs must have the same size');
    end
    
    % Compute L2 error
    diff = a(:) - b(:);
    err = sqrt(sum(abs(diff).^2));
end
