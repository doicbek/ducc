% Test script for DUCC0 SHT MEX functions
% This script tests synthesis_2d, analysis_2d, and get_gridweights

function test_sht()
    % Test SHT MEX functions
    fprintf('Testing DUCC0 SHT MEX functions...\n\n');
    
    % Add paths
    addpath(fullfile(fileparts(mfilename('fullpath')), '..'));
    addpath(fullfile(fileparts(mfilename('fullpath')), 'build'));
    
    % Test parameters
    lmax = 32;
    mmax = lmax;
    spin = 0;
    geometry = 'CC';
    ntheta = lmax + 2;  % CC geometry needs lmax+2 rings
    nphi = 2 * mmax + 2;
    
    fprintf('Test parameters:\n');
    fprintf('  lmax: %d\n', lmax);
    fprintf('  mmax: %d\n', mmax);
    fprintf('  spin: %d\n', spin);
    fprintf('  geometry: %s\n', geometry);
    fprintf('  ntheta: %d\n', ntheta);
    fprintf('  nphi: %d\n', nphi);
    fprintf('\n');
    
    % Test 1: get_gridweights
    fprintf('Test 1: get_gridweights\n');
    try
        weights = ducc0.sht.get_gridweights(geometry, ntheta);
        fprintf('  ✓ get_gridweights succeeded\n');
        fprintf('  Size: %d\n', length(weights));
        fprintf('  All positive: %s\n', mat2str(all(weights > 0)));
        fprintf('  Sum: %f\n', sum(weights));
        fprintf('  Min: %e, Max: %e\n', min(weights), max(weights));
    catch ME
        fprintf('  ✗ get_gridweights failed: %s\n', ME.message);
        return;
    end
    fprintf('\n');
    
    % Test 2: synthesis_2d with 1D alm
    fprintf('Test 2: synthesis_2d (1D alm)\n');
    try
        nalm = ((lmax+1)*(lmax+2))/2;
        alm = randn(1, nalm) + 1i*randn(1, nalm);
        % Make m=0 modes real
        alm(1:lmax+1) = real(alm(1:lmax+1));
        
        map = ducc0.sht.synthesis_2d(alm, lmax, ...
            'spin', spin, ...
            'geometry', geometry, ...
            'ntheta', ntheta, ...
            'nphi', nphi);
        
        fprintf('  ✓ synthesis_2d succeeded\n');
        fprintf('  Input alm size: [%d]\n', length(alm));
        fprintf('  Output map size: [%s]\n', mat2str(size(map)));
        fprintf('  Map is real: %s\n', mat2str(isreal(map)));
        fprintf('  Map min: %f, max: %f\n', min(map(:)), max(map(:)));
    catch ME
        fprintf('  ✗ synthesis_2d failed: %s\n', ME.message);
        fprintf('  Error details: %s\n', getReport(ME));
        return;
    end
    fprintf('\n');
    
    % Test 3: analysis_2d (round-trip)
    fprintf('Test 3: analysis_2d (round-trip)\n');
    try
        alm2 = ducc0.sht.analysis_2d(map, lmax, ...
            'spin', spin, ...
            'geometry', geometry);
        
        fprintf('  ✓ analysis_2d succeeded\n');
        fprintf('  Output alm size: [%s]\n', mat2str(size(alm2)));
        fprintf('  Input map size: [%s]\n', mat2str(size(map)));
        
        % Check round-trip error
        error = max(abs(alm(:) - alm2(:)));
        fprintf('  Round-trip error: %e\n', error);
        if error < 1e-10
            fprintf('  ✓ Round-trip error is acceptable (< 1e-10)\n');
        else
            fprintf('  ✗ Round-trip error is too large (>= 1e-10)\n');
        end
    catch ME
        fprintf('  ✗ analysis_2d failed: %s\n', ME.message);
        fprintf('  Error details: %s\n', getReport(ME));
        return;
    end
    fprintf('\n');
    
    % Test 4: synthesis_2d with 2D alm
    fprintf('Test 4: synthesis_2d (2D alm)\n');
    try
        nalm = ((lmax+1)*(lmax+2))/2;
        alm = randn(1, nalm) + 1i*randn(1, nalm);
        alm(1:lmax+1) = real(alm(1:lmax+1));
        alm = reshape(alm, [1, nalm]);  % Ensure 2D
        
        map = ducc0.sht.synthesis_2d(alm, lmax, ...
            'spin', spin, ...
            'geometry', geometry, ...
            'ntheta', ntheta, ...
            'nphi', nphi);
        
        fprintf('  ✓ synthesis_2d (2D alm) succeeded\n');
        fprintf('  Input alm size: [%s]\n', mat2str(size(alm)));
        fprintf('  Output map size: [%s]\n', mat2str(size(map)));
    catch ME
        fprintf('  ✗ synthesis_2d (2D alm) failed: %s\n', ME.message);
        fprintf('  Error details: %s\n', getReport(ME));
        return;
    end
    fprintf('\n');
    
    % Test 5: Different geometries
    fprintf('Test 5: Different geometries\n');
    geometries = {'CC', 'F1', 'MW', 'GL', 'DH'};
    for i = 1:length(geometries)
        geo = geometries{i};
        try
            % Get appropriate ntheta for geometry
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
        end
    end
    fprintf('\n');
    
    % Test 6: Spin-2 fields
    fprintf('Test 6: Spin-2 fields\n');
    try
        spin2 = 2;
        nalm = ((lmax+1)*(lmax+2))/2;
        alm = randn(2, nalm) + 1i*randn(2, nalm);
        alm(:, 1:lmax+1) = real(alm(:, 1:lmax+1));
        
        map = ducc0.sht.synthesis_2d(alm, lmax, ...
            'spin', spin2, ...
            'geometry', geometry, ...
            'ntheta', ntheta, ...
            'nphi', nphi);
        
        fprintf('  ✓ Spin-2 synthesis succeeded\n');
        fprintf('  Input alm size: [%s]\n', mat2str(size(alm)));
        fprintf('  Output map size: [%s]\n', mat2str(size(map)));
        fprintf('  Expected map components: 2\n');
        fprintf('  Actual map components: %d\n', size(map, 1));
    catch ME
        fprintf('  ✗ Spin-2 synthesis failed: %s\n', ME.message);
        fprintf('  Error details: %s\n', getReport(ME));
    end
    fprintf('\n');
    
    % Test 7: Single precision
    fprintf('Test 7: Single precision\n');
    try
        nalm = ((lmax+1)*(lmax+2))/2;
        alm = single(randn(1, nalm) + 1i*randn(1, nalm));
        alm(1:lmax+1) = real(alm(1:lmax+1));
        
        map = ducc0.sht.synthesis_2d(alm, lmax, ...
            'spin', spin, ...
            'geometry', geometry, ...
            'ntheta', ntheta, ...
            'nphi', nphi);
        
        fprintf('  ✓ Single precision synthesis succeeded\n');
        fprintf('  Input type: %s\n', class(alm));
        fprintf('  Output type: %s\n', class(map));
    catch ME
        fprintf('  ✗ Single precision synthesis failed: %s\n', ME.message);
        fprintf('  Error details: %s\n', getReport(ME));
    end
    fprintf('\n');
    
    % Test 8: Edge cases
    fprintf('Test 8: Edge cases\n');
    
    % Small lmax
    try
        lmax_small = 4;
        nalm_small = ((lmax_small+1)*(lmax_small+2))/2;
        alm_small = randn(1, nalm_small) + 1i*randn(1, nalm_small);
        alm_small(1:lmax_small+1) = real(alm_small(1:lmax_small+1));
        
        map_small = ducc0.sht.synthesis_2d(alm_small, lmax_small, ...
            'spin', 0, ...
            'geometry', 'CC');
        
        fprintf('  ✓ Small lmax (%d) succeeded\n', lmax_small);
    catch ME
        fprintf('  ✗ Small lmax failed: %s\n', ME.message);
    end
    
    % Different mmax
    try
        mmax_diff = 16;
        nalm_diff = ((lmax+1)*(lmax+2))/2 + (lmax+1)*(mmax_diff-lmax);
        alm_diff = randn(1, nalm_diff) + 1i*randn(1, nalm_diff);
        alm_diff(1:lmax+1) = real(alm_diff(1:lmax+1));
        
        map_diff = ducc0.sht.synthesis_2d(alm_diff, lmax, ...
            'spin', 0, ...
            'geometry', 'CC', ...
            'mmax', mmax_diff);
        
        fprintf('  ✓ Different mmax (%d) succeeded\n', mmax_diff);
    catch ME
        fprintf('  ✗ Different mmax failed: %s\n', ME.message);
    end
    
    fprintf('\n');
    
    % Summary
    fprintf('Test Summary\n');
    fprintf('============\n');
    fprintf('All basic tests completed.\n');
    fprintf('Check output above for any failures.\n');
    fprintf('\n');
end

