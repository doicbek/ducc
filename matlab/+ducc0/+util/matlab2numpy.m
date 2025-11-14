function py_arr = matlab2numpy(mat_arr)
%MATLAB2NUMPY Convert MATLAB array to Python numpy array
%   PY_ARR = MATLAB2NUMPY(MAT_ARR) converts a MATLAB numeric array to a
%   Python numpy array.
%
%   This is an internal utility function used by DUCC0 wrappers.

    if isempty(mat_arr)
        py_arr = py.numpy.array(py.list());
        return;
    end
    
    % Handle complex arrays
    if ~isreal(mat_arr)
        % Convert to Python complex array
        py_arr = py.numpy.array(mat_arr, pyargs('dtype', 'complex128'));
    else
        % Convert to appropriate numpy type
        if isa(mat_arr, 'double')
            py_arr = py.numpy.array(mat_arr, pyargs('dtype', 'float64'));
        elseif isa(mat_arr, 'single')
            py_arr = py.numpy.array(mat_arr, pyargs('dtype', 'float32'));
        elseif isa(mat_arr, 'int32')
            py_arr = py.numpy.array(mat_arr, pyargs('dtype', 'int32'));
        elseif isa(mat_arr, 'int64')
            py_arr = py.numpy.array(mat_arr, pyargs('dtype', 'int64'));
        else
            % Default to double
            py_arr = py.numpy.array(double(mat_arr), pyargs('dtype', 'float64'));
        end
    end
end

