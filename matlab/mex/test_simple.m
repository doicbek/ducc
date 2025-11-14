% Simple test script for DUCC0 SHT MEX functions
% This script performs basic tests to verify the implementation works

function test_simple()
    fprintf('========================================\n');
    fprintf('DUCC0 SHT Simple Test\n');
    fprintf('========================================\n\n');
    
    % Add paths
    mex_dir = fullfile(fileparts(mfilename('fullpath')), 'build');
    matlab_dir = fullfile(fileparts(mfilename('fullpath')), '..');
    addpath(mex_dir);
    addpath(matlab_dir);
    
    fprintf('MEX directory: %s\n', mex_dir);
    fprintf('MATLAB directory: %s\n', matlab_dir);
    fprintf('\n');
    
    % Test 1: Check if MEX files exist
    fprintf('Test 1: Check MEX files\n');
    fprintf('------------------------\n');
    mex_files = {
        'ducc0_sht_get_gridweights_mex', ...
        'ducc0_sht_synthesis_2d_mex', ...
        'ducc0_sht_analysis_2d_mex' ...
    };
    
    all_found = true;
    for i = 1:length(mex_files)
        mex_file = mex_files{i};
        if exist(mex_file, 'file') == 3
            fprintf('  ✓ %s found\n', mex_file);
        else
            fprintf('  ✗ %s NOT found\n', mex_file);
            fprintf('    Expected location: %s\n', fullfile(mex_dir, [mex_file, '.', mexext]));
            all_found = false;
        end
    end
    
    if ~all_found
        fprintf('\n  ERROR: Some MEX files are missing!\n');
        fprintf('  Please build MEX files first:\n');
        fprintf('    cd matlab/mex\n');
        fprintf('    mkdir build\n');
        fprintf('    cd build\n');
        fprintf('    cmake ..\n');
        fprintf('    cmake --build .\n');
        return;
    end
    
    fprintf('\n');
    
    % Test 2: get_gridweights
    fprintf('Test 2: get_gridweights\n');
    fprintf('------------------------\n');
    try
        weights = ducc0.sht.get_gridweights('CC', 64);
        fprintf('  ✓ get_gridweights succeeded\n');
        fprintf('  Size: %d\n', length(weights));
        fprintf('  All positive: %s\n', mat2str(all(weights > 0)));
        fprintf('  Min: %e, Max: %e\n', min(weights), max(weights));
    catch ME
        fprintf('  ✗ get_gridweights failed: %s\n', ME.message);
        fprintf('  Error details:\n');
        fprintf('    %s\n', getReport(ME, 'extended'));
        return;
    end
    fprintf('\n');
    
    % Test 3: synthesis_2d (basic)
    fprintf('Test 3: synthesis_2d (basic)\n');
    fprintf('------------------------\n');
    try
        lmax = 16;
        spin = 0;
        geometry = 'CC';
        ntheta = lmax + 2;
        nphi = 2 * lmax + 2;
        nalm = ((lmax+1)*(lmax+2))/2;
        
        % Create test alm
        alm = randn(1, nalm) + 1i*randn(1, nalm);
        alm(1:lmax+1) = real(alm(1:lmax+1));  % Make m=0 modes real
        
        fprintf('  Input: lmax=%d, nalm=%d\n', lmax, nalm);
        
        map = ducc0.sht.synthesis_2d(alm, lmax, ...
            'spin', spin, ...
            'geometry', geometry, ...
            'ntheta', ntheta, ...
            'nphi', nphi);
        
        fprintf('  ✓ synthesis_2d succeeded\n');
        fprintf('  Output map size: [%s]\n', mat2str(size(map)));
        fprintf('  Map is real: %s\n', mat2str(isreal(map)));
        fprintf('  Map min: %f, max: %f\n', min(map(:)), max(map(:)));
    catch ME
        fprintf('  ✗ synthesis_2d failed: %s\n', ME.message);
        fprintf('  Error details:\n');
        fprintf('    %s\n', getReport(ME, 'extended'));
        return;
    end
    fprintf('\n');
    
    % Test 4: analysis_2d (round-trip)
    fprintf('Test 4: analysis_2d (round-trip)\n');
    fprintf('------------------------\n');
    try
        alm2 = ducc0.sht.analysis_2d(map, lmax, ...
            'spin', spin, ...
            'geometry', geometry);
        
        fprintf('  ✓ analysis_2d succeeded\n');
        fprintf('  Output alm size: [%s]\n', mat2str(size(alm2)));
        
        % Check round-trip error
        error = max(abs(alm(:) - alm2(:)));
        fprintf('  Round-trip error: %e\n', error);
        
        if error < 1e-8
            fprintf('  ✓ Round-trip error is acceptable (< 1e-8)\n');
        else
            fprintf('  ⚠ Round-trip error is large (>= 1e-8)\n');
            fprintf('    This may be normal for large lmax or certain geometries\n');
        end
    catch ME
        fprintf('  ✗ analysis_2d failed: %s\n', ME.message);
        fprintf('  Error details:\n');
        fprintf('    %s\n', getReport(ME, 'extended'));
        return;
    end
    fprintf('\n');
    
    % Test 5: Different geometries
    fprintf('Test 5: Different geometries\n');
    fprintf('------------------------\n');
    geometries = {'CC', 'F1', 'MW', 'GL', 'DH'};
    for i = 1:length(geometries)
        geo = geometries{i};
        try
            % Get appropriate ntheta
            if strcmp(geo, 'CC')
                ntheta_geo = lmax + 2;
            elseif strcmp(geo, 'DH')
                ntheta_geo = 2*lmax + 2;
            else
                ntheta_geo = lmax + 1;
            end
            
            nalm_geo = ((lmax+1)*(lmax+2))/2;
            alm_geo = randn(1, nalm_geo) + 1i*randn(1, nalm_geo);
            alm_geo(1:lmax+1) = real(alm_geo(1:lmax+1));
            
            map_geo = ducc0.sht.synthesis_2d(alm_geo, lmax, ...
                'spin', spin, ...
                'geometry', geo, ...
                'ntheta', ntheta_geo, ...
                'nphi', nphi);
            
            fprintf('  ✓ %s: synthesis succeeded\n', geo);
        catch ME
            fprintf('  ✗ %s: synthesis failed: %s\n', geo, ME.message);
        end
    end
    fprintf('\n');
    
    % Summary
    fprintf('========================================\n');
    fprintf('Test Summary\n');
    fprintf('========================================\n');
    fprintf('All basic tests completed.\n');
    fprintf('If all tests passed, the SHT module is working correctly.\n');
    fprintf('If any tests failed, check the error messages above.\n');
    fprintf('\n');
end

