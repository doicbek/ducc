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

    py_a = ducc0.util.matlab2numpy(a);
    py_b = ducc0.util.matlab2numpy(b);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.misc.vdot(py_a, py_b);
    
    % Convert result (scalar)
    if isa(py_result, 'py.complex')
        result = complex(double(py_result.real), double(py_result.imag));
    else
        result = double(py_result);
    end
end

