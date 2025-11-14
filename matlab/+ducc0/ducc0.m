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

function mod_info = ducc0()
    %DUCC0 Get DUCC module information
    %   Returns information about the DUCC MEX interface
    
    persistent info;
    if isempty(info)
        info = struct();
        info.version = '0.1.0';
        info.interface = 'MEX';
        info.description = 'DUCC MATLAB MEX Interface - Direct C++ access without Python';
    end
    mod_info = info;
end
