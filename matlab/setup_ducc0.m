% setup_ducc0  Configure MATLAB path for the DUCC0 wrapper.
%
%   Run this script once from the MATLAB command window (or add it to your
%   startup.m) to make the ducc0 package available in your session.
%
%   Usage:
%     >> run('/path/to/ducc/matlab/setup_ducc0.m')
%
%   Or, to make the setup permanent, add the following line to your
%   startup.m file (usually ~/Documents/MATLAB/startup.m):
%
%     run('/path/to/ducc/matlab/setup_ducc0.m');
%
%   After setup, use the library as:
%     >> y = ducc0.fft.c2c(x);
%     >> map = ducc0.sht.synthesis_2d(alm, lmax);

this_dir = fileparts(mfilename('fullpath'));

% Add the +ducc0 package directory
addpath(this_dir);

% Locate and add the MEX binary directory.
% The expected location after a standard CMake build is matlab/mex/build/.
mex_build = fullfile(this_dir, 'mex', 'build');

if exist(mex_build, 'dir')
    addpath(mex_build);
else
    warning('ducc0:setup:mexNotFound', ...
        ['MEX binaries not found in %s.\n' ...
         'Build them first:\n' ...
         '  cd %s\n' ...
         '  mkdir build && cd build\n' ...
         '  cmake ..\n' ...
         '  cmake --build . -j$(nproc)\n' ...
         'Then re-run setup_ducc0.m.'], ...
        mex_build, fullfile(this_dir, 'mex'));
    return;
end

% Quick sanity check: verify a representative MEX file is reachable.
if exist('ducc0_fft_c2c_mex', 'file') ~= 3
    warning('ducc0:setup:mexNotOnPath', ...
        'MEX binaries were found in %s but cannot be loaded.\n%s', ...
        mex_build, ...
        'Ensure the MEX files were compiled for this platform.');
    return;
end

fprintf('ducc0: paths configured.\n');
fprintf('  package : %s\n', this_dir);
fprintf('  MEX     : %s\n', mex_build);
fprintf('Try: ducc0.fft.good_size(1000)\n');
