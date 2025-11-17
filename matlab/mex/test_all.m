% Comprehensive test script for DUCC0 MATLAB MEX interface
% This script tests all implemented MEX functions

function results = test_all()
    % Run all tests and return results
    results = struct();
    
    fprintf('========================================\n');
    fprintf('DUCC0 MATLAB MEX Interface Test Suite\n');
    fprintf('========================================\n\n');
    
    % Add paths
    mex_dir = fullfile(fileparts(mfilename('fullpath')), 'build');
    matlab_dir = fullfile(fileparts(mfilename('fullpath')), '..');
    addpath(mex_dir);
    addpath(matlab_dir);
    
    fprintf('MEX directory: %s\n', mex_dir);
    fprintf('MATLAB directory: %s\n', matlab_dir);
    fprintf('\n');
    
    % Test 1: Check MEX files
    fprintf('Test 1: Checking MEX files...\n');
    results.mex_files = test_mex_files();
    fprintf('\n');
    
    % Test 2: Test FFT functions
    fprintf('Test 2: Testing FFT functions...\n');
    results.fft = test_fft();
    fprintf('\n');
    
    % Test 3: Test SHT functions
    fprintf('Test 3: Testing SHT functions...\n');
    results.sht = test_sht_functions();
    fprintf('\n');
    
    % Test 4: Test HEALPix functions
    fprintf('Test 4: Testing HEALPix functions...\n');
    results.healpix = test_healpix();
    fprintf('\n');
    
    % Test 5: Test Misc functions
    fprintf('Test 5: Testing Misc functions...\n');
    results.misc = test_misc();
    fprintf('\n');
    
    % Summary
    fprintf('========================================\n');
    fprintf('Test Summary\n');
    fprintf('========================================\n');
    print_summary(results);
    fprintf('\n');
end

function result = test_mex_files()
    % Check if MEX files exist
    result = struct();
    result.passed = true;
    result.failed = {};
    
    mex_files = {
        'ducc0_fft_c2c_mex', ...
        'ducc0_fft_good_size_mex', ...
        'ducc0_sht_synthesis_2d_mex', ...
        'ducc0_sht_analysis_2d_mex', ...
        'ducc0_sht_get_gridweights_mex' ...
    };
    
    for i = 1:length(mex_files)
        mex_file = mex_files{i};
        if exist(mex_file, 'file') == 3
            fprintf('  ✓ %s found\n', mex_file);
        else
            fprintf('  ✗ %s NOT found\n', mex_file);
            result.passed = false;
            result.failed{end+1} = mex_file;
        end
    end
end

function result = test_fft()
    % Test FFT functions
    result = struct();
    result.passed = true;
    result.failed = {};
    result.errors = {};
    
    % Test c2c
    try
        x = randn(64, 64) + 1i*randn(64, 64);
        y = ducc0.fft.c2c(x);
        z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);
        error = max(abs(x(:) - z(:)));
        fprintf('  ✓ c2c: round-trip error = %e\n', error);
        if error > 1e-10
            result.passed = false;
            result.errors{end+1} = sprintf('c2c: round-trip error too large: %e', error);
        end
    catch ME
        fprintf('  ✗ c2c failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'c2c';
        result.errors{end+1} = ME.message;
    end
    
    % Test good_size
    try
        n = 1000;
        n_good = ducc0.fft.good_size(n);
        fprintf('  ✓ good_size: %d -> %d\n', n, n_good);
        if n_good < n
            result.passed = false;
            result.errors{end+1} = sprintf('good_size: returned value < input: %d < %d', n_good, n);
        end
    catch ME
        fprintf('  ✗ good_size failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'good_size';
        result.errors{end+1} = ME.message;
    end
end

function result = test_sht_functions()
    % Test SHT functions
    result = struct();
    result.passed = true;
    result.failed = {};
    result.errors = {};
    
    lmax = 32;
    spin = 0;
    geometry = 'CC';
    ntheta = lmax + 2;
    nphi = 2 * lmax + 2;
    
    % Test get_gridweights
    try
        weights = ducc0.sht.get_gridweights(geometry, ntheta);
        fprintf('  ✓ get_gridweights: size = %d\n', length(weights));
        if ~all(weights > 0)
            result.passed = false;
            result.errors{end+1} = 'get_gridweights: some weights are not positive';
        end
    catch ME
        fprintf('  ✗ get_gridweights failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'get_gridweights';
        result.errors{end+1} = ME.message;
        return;  % Can't continue without get_gridweights
    end
    
    % Test synthesis_2d
    try
        nalm = ((lmax+1)*(lmax+2))/2;
        alm = randn(1, nalm) + 1i*randn(1, nalm);
        alm(1:lmax+1) = real(alm(1:lmax+1));
        
        map = ducc0.sht.synthesis_2d(alm, lmax, ...
            'spin', spin, ...
            'geometry', geometry, ...
            'ntheta', ntheta, ...
            'nphi', nphi);
        
        fprintf('  ✓ synthesis_2d: map size = [%s]\n', mat2str(size(map)));
        if ~isreal(map)
            result.passed = false;
            result.errors{end+1} = 'synthesis_2d: map should be real';
        end
    catch ME
        fprintf('  ✗ synthesis_2d failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'synthesis_2d';
        result.errors{end+1} = ME.message;
        return;  % Can't continue without synthesis_2d
    end
    
    % Test analysis_2d (round-trip)
    try
        alm2 = ducc0.sht.analysis_2d(map, lmax, ...
            'spin', spin, ...
            'geometry', geometry);
        
        error = max(abs(alm(:) - alm2(:)));
        fprintf('  ✓ analysis_2d: round-trip error = %e\n', error);
        if error > 1e-8
            result.passed = false;
            result.errors{end+1} = sprintf('analysis_2d: round-trip error too large: %e', error);
        end
    catch ME
        fprintf('  ✗ analysis_2d failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'analysis_2d';
        result.errors{end+1} = ME.message;
    end
    
    % Test different geometries
    geometries = {'CC', 'F1', 'MW', 'GL', 'DH'};
    for i = 1:length(geometries)
        geo = geometries{i};
        try
            if strcmp(geo, 'CC')
                ntheta_geo = lmax + 2;
            elseif strcmp(geo, 'DH')
                ntheta_geo = 2*lmax + 2;
            else
                ntheta_geo = lmax + 1;
            end
            
            nalm = ((lmax+1)*(lmax+2))/2;
            alm = randn(1, nalm) + 1i*randn(1, nalm);
            alm(1:lmax+1) = real(alm(1:lmax+1));
            
            map = ducc0.sht.synthesis_2d(alm, lmax, ...
                'spin', spin, ...
                'geometry', geo, ...
                'ntheta', ntheta_geo, ...
                'nphi', nphi);
            
            fprintf('  ✓ %s: synthesis succeeded\n', geo);
        catch ME
            fprintf('  ✗ %s: synthesis failed: %s\n', geo, ME.message);
            result.errors{end+1} = sprintf('%s: %s', geo, ME.message);
        end
    end
end

function result = test_healpix()
    % Test HEALPix functions
    result = struct();
    result.passed = true;
    result.failed = {};
    result.errors = {};
    
    % Test nside2npix
    try
        nside = 256;
        npix = ducc0.healpix.nside2npix(nside);
        expected = 12 * nside * nside;
        fprintf('  ✓ nside2npix: %d -> %d (expected: %d)\n', nside, npix, expected);
        if npix ~= expected
            result.passed = false;
            result.errors{end+1} = sprintf('nside2npix: incorrect result: %d != %d', npix, expected);
        end
    catch ME
        fprintf('  ✗ nside2npix failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'nside2npix';
        result.errors{end+1} = ME.message;
    end
    
    % Test npix2nside
    try
        npix = 12 * 256 * 256;
        nside = ducc0.healpix.npix2nside(npix);
        expected = 256;
        fprintf('  ✓ npix2nside: %d -> %d (expected: %d)\n', npix, nside, expected);
        if nside ~= expected
            result.passed = false;
            result.errors{end+1} = sprintf('npix2nside: incorrect result: %d != %d', nside, expected);
        end
    catch ME
        fprintf('  ✗ npix2nside failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'npix2nside';
        result.errors{end+1} = ME.message;
    end
end

function result = test_misc()
    % Test Misc functions
    result = struct();
    result.passed = true;
    result.failed = {};
    result.errors = {};
    
    % Test l2error
    try
        a = randn(128, 128);
        b = a + 1e-10 * randn(128, 128);
        error = ducc0.misc.l2error(a, b);
        fprintf('  ✓ l2error: error = %e\n', error);
        if error > 1e-9
            result.passed = false;
            result.errors{end+1} = sprintf('l2error: error too large: %e', error);
        end
    catch ME
        fprintf('  ✗ l2error failed: %s\n', ME.message);
        result.passed = false;
        result.failed{end+1} = 'l2error';
        result.errors{end+1} = ME.message;
    end
end

function print_summary(results)
    % Print test summary
    fprintf('MEX Files: ');
    if results.mex_files.passed
        fprintf('PASSED\n');
    else
        fprintf('FAILED (%d files missing)\n', length(results.mex_files.failed));
    end
    
    fprintf('FFT: ');
    if results.fft.passed
        fprintf('PASSED\n');
    else
        fprintf('FAILED (%d functions failed)\n', length(results.fft.failed));
    end
    
    fprintf('SHT: ');
    if results.sht.passed
        fprintf('PASSED\n');
    else
        fprintf('FAILED (%d functions failed)\n', length(results.sht.failed));
    end
    
    fprintf('HEALPix: ');
    if results.healpix.passed
        fprintf('PASSED\n');
    else
        fprintf('FAILED (%d functions failed)\n', length(results.healpix.failed));
    end
    
    fprintf('Misc: ');
    if results.misc.passed
        fprintf('PASSED\n');
    else
        fprintf('FAILED (%d functions failed)\n', length(results.misc.failed));
    end
    
    % Print errors
    if ~isempty(results.fft.errors)
        fprintf('\nFFT Errors:\n');
        for i = 1:length(results.fft.errors)
            fprintf('  - %s\n', results.fft.errors{i});
        end
    end
    
    if ~isempty(results.sht.errors)
        fprintf('\nSHT Errors:\n');
        for i = 1:length(results.sht.errors)
            fprintf('  - %s\n', results.sht.errors{i});
        end
    end
    
    if ~isempty(results.healpix.errors)
        fprintf('\nHEALPix Errors:\n');
        for i = 1:length(results.healpix.errors)
            fprintf('  - %s\n', results.healpix.errors{i});
        end
    end
    
    if ~isempty(results.misc.errors)
        fprintf('\nMisc Errors:\n');
        for i = 1:length(results.misc.errors)
            fprintf('  - %s\n', results.misc.errors{i});
        end
    end
end



