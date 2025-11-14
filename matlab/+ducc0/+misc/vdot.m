function result = vdot(a, b)
%VDOT Compute scalar product of two arrays
%   RESULT = VDOT(A, B) computes sum_i(conj(a_i)*b_i) over all array elements.
%
%   Parameters
%   ----------
%   a : numeric scalar or array
%       First array (or scalar)
%   b : numeric scalar or array
%       Second array (or scalar), same shape as a
%
%   Returns
%   -------
%   result : double or complex
%       The scalar product. If result can be represented as float, returns float,
%       otherwise returns complex.
%
%   Notes
%   -----
%   Accumulation is performed in long double precision for good accuracy.

    if ~isnumeric(a) || ~isnumeric(b)
        error('DUCC0:Misc:VDOT:InputError', 'a and b must be numeric');
    end
    
    if ~isequal(size(a), size(b))
        error('DUCC0:Misc:VDOT:InputError', 'a and b must have the same shape');
    end
    
    result = ducc0_misc_vdot_mex(a, b);
end
