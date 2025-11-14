%DUCC0 Main DUCC module for MATLAB
%   This module provides access to the DUCC (Distinctly Useful Code Collection)
%   library, which includes Fast Fourier Transforms, Spherical Harmonic Transforms,
%   non-uniform FFTs, and other numerical computation tools.
%
%   Usage:
%     import ducc0.*
%     result = ducc0.fft.c2c(data);
%
%   See also: ducc0.fft, ducc0.sht, ducc0.nufft, ducc0.healpix, ducc0.misc

function py_mod = ducc0()
    %DUCC0 Get the Python ducc0 module
    %   Returns the Python ducc0 module for direct access if needed
    
    persistent py_ducc0;
    if isempty(py_ducc0)
        try
            py_ducc0 = py.importlib.import_module('ducc0');
        catch ME
            error('DUCC0:PythonModuleNotFound', ...
                ['Failed to import ducc0 Python module. ' ...
                 'Please ensure ducc0 is installed: pip install ducc0']);
        end
    end
    py_mod = py_ducc0;
end

