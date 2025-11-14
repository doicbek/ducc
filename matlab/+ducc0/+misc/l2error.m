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

    py_a = ducc0.util.matlab2numpy(a);
    py_b = ducc0.util.matlab2numpy(b);
    
    py_mod = ducc0.ducc0();
    py_result = py_mod.misc.l2error(py_a, py_b);
    err = double(py_result);
end

