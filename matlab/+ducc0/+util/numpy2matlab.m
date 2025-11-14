function mat_arr = numpy2matlab(py_arr)
%NUMPY2MATLAB Convert Python numpy array to MATLAB array
%   MAT_ARR = NUMPY2MATLAB(PY_ARR) converts a Python numpy array to a
%   MATLAB numeric array.
%
%   This is an internal utility function used by DUCC0 wrappers.

    if isempty(py_arr) || py_arr.size == 0
        mat_arr = [];
        return;
    end
    
    % Get shape
    try
        shape_cell = cell(py_arr.shape);
        shape = cellfun(@double, shape_cell);
        shape = shape(:)';  % Ensure row vector
    catch
        shape = [];
    end
    
    % Check if complex
    try
        dtype_str = char(py_arr.dtype.name);
        is_complex = contains(dtype_str, 'complex');
    catch
        is_complex = false;
    end
    
    % Try direct conversion first (fastest)
    try
        if is_complex
            % For complex arrays, extract real and imaginary parts
            real_part = double(py_arr.real);
            imag_part = double(py_arr.imag);
            mat_arr = complex(real_part, imag_part);
        else
            mat_arr = double(py_arr);
        end
        
        % Handle shape - Python uses row-major, MATLAB uses column-major
        if ~isempty(shape) && length(shape) > 1
            % Reshape and permute to convert from row-major to column-major
            mat_arr = reshape(mat_arr, fliplr(shape));
            mat_arr = permute(mat_arr, length(shape):-1:1);
        elseif ~isempty(shape) && length(shape) == 1
            mat_arr = mat_arr(:);  % Column vector
        end
    catch ME
        % Fallback: use tolist() method
        try
            py_list = py_arr.tolist();
            mat_arr = py_list;
            % Convert nested lists to MATLAB array
            mat_arr = convert_python_nested_list(py_list, shape, is_complex);
        catch ME2
            error('DUCC0:ConversionError', ...
                'Failed to convert Python array to MATLAB: %s\nOriginal error: %s', ...
                ME2.message, ME.message);
        end
    end
end

function mat_arr = convert_python_nested_list(py_list, shape, is_complex)
    % Helper to convert nested Python lists to MATLAB arrays
    if isa(py_list, 'py.list')
        % Convert to cell array first
        n = length(py_list);
        cell_arr = cell(n, 1);
        for i = 1:n
            cell_arr{i} = convert_python_nested_list(py_list{i}, [], is_complex);
        end
        % Try to convert to numeric array
        try
            if is_complex
                mat_arr = cellfun(@(x) complex(double(x.real), double(x.imag)), cell_arr);
            else
                mat_arr = cellfun(@double, cell_arr);
            end
        catch
            mat_arr = cell_arr;
        end
    elseif isa(py_list, 'py.complex')
        mat_arr = complex(double(py_list.real), double(py_list.imag));
    else
        mat_arr = double(py_list);
    end
    
    % Reshape if shape is provided
    if ~isempty(shape) && length(shape) > 1
        mat_arr = reshape(mat_arr, fliplr(shape));
        mat_arr = permute(mat_arr, length(shape):-1:1);
    end
end

