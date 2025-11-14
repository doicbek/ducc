function py_arr = matlab2numpy(mat_arr)
%MATLAB2NUMPY Convert MATLAB array to Python numpy array
%   This function is deprecated. The MEX interface does not use Python.
%
%   This function is no longer needed as the MEX interface works directly
%   with MATLAB arrays without Python conversion.

    error('DUCC0:Deprecated', ...
        'matlab2numpy is deprecated. The MEX interface does not use Python.');
end
