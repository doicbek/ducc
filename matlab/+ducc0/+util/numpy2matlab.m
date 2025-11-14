function mat_arr = numpy2matlab(py_arr)
%NUMPY2MATLAB Convert Python numpy array to MATLAB array
%   This function is deprecated. The MEX interface does not use Python.
%
%   This function is no longer needed as the MEX interface works directly
%   with MATLAB arrays without Python conversion.

    error('DUCC0:Deprecated', ...
        'numpy2matlab is deprecated. The MEX interface does not use Python.');
end
