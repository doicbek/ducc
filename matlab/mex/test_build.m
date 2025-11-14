% Test script to verify MEX files can be loaded
% This script checks if MEX files are available and can be called

function test_build()
    % Test if MEX files are built and available
    fprintf('Testing DUCC0 MEX file availability...\n\n');
    
    % Add paths
    mex_dir = fullfile(fileparts(mfilename('fullpath')), 'build');
    addpath(mex_dir);
    addpath(fullfile(fileparts(mfilename('fullpath')), '..'));
    
    fprintf('MEX directory: %s\n', mex_dir);
    fprintf('\n');
    
    % Check if MEX files exist
    mex_files = {
        'ducc0_fft_c2c_mex', ...
        'ducc0_fft_good_size_mex', ...
        'ducc0_sht_synthesis_2d_mex', ...
        'ducc0_sht_analysis_2d_mex', ...
        'ducc0_sht_get_gridweights_mex' ...
    };
    
    fprintf('Checking MEX files:\n');
    for i = 1:length(mex_files)
        mex_file = mex_files{i};
        if exist(mex_file, 'file') == 3
            fprintf('  ✓ %s found\n', mex_file);
        else
            fprintf('  ✗ %s NOT found\n', mex_file);
            fprintf('    Location: %s\n', fullfile(mex_dir, [mex_file, '.', mexext]));
        end
    end
    fprintf('\n');
    
    % Try to call MEX functions directly
    fprintf('Testing MEX function calls:\n');
    
    % Test ducc0_fft_good_size_mex
    try
        n = 1000;
        n_good = ducc0_fft_good_size_mex(n);
        fprintf('  ✓ ducc0_fft_good_size_mex: %d -> %d\n', n, n_good);
    catch ME
        fprintf('  ✗ ducc0_fft_good_size_mex failed: %s\n', ME.message);
    end
    
    % Test ducc0_sht_get_gridweights_mex
    try
        weights = ducc0_sht_get_gridweights_mex('CC', 64);
        fprintf('  ✓ ducc0_sht_get_gridweights_mex: size = %d\n', length(weights));
    catch ME
        fprintf('  ✗ ducc0_sht_get_gridweights_mex failed: %s\n', ME.message);
    end
    
    % Test ducc0_fft_c2c_mex
    try
        x = randn(64, 64) + 1i*randn(64, 64);
        y = ducc0_fft_c2c_mex(x, [], true, 0, 0);
        fprintf('  ✓ ducc0_fft_c2c_mex: input size [%s], output size [%s]\n', ...
            mat2str(size(x)), mat2str(size(y)));
    catch ME
        fprintf('  ✗ ducc0_fft_c2c_mex failed: %s\n', ME.message);
    end
    
    fprintf('\n');
    
    % Check MATLAB wrapper functions
    fprintf('Checking MATLAB wrapper functions:\n');
    wrapper_functions = {
        'ducc0.fft.c2c', ...
        'ducc0.fft.good_size', ...
        'ducc0.sht.synthesis_2d', ...
        'ducc0.sht.analysis_2d', ...
        'ducc0.sht.get_gridweights' ...
    };
    
    for i = 1:length(wrapper_functions)
        func_name = wrapper_functions{i};
        if exist(func_name, 'file') == 2
            fprintf('  ✓ %s found\n', func_name);
        else
            fprintf('  ✗ %s NOT found\n', func_name);
        end
    end
    fprintf('\n');
    
    % Summary
    fprintf('Build Test Summary\n');
    fprintf('==================\n');
    fprintf('Check output above for any missing files or failures.\n');
    fprintf('If MEX files are missing, build them using CMake:\n');
    fprintf('  cd matlab/mex\n');
    fprintf('  mkdir build\n');
    fprintf('  cd build\n');
    fprintf('  cmake ..\n');
    fprintf('  cmake --build .\n');
    fprintf('\n');
end

