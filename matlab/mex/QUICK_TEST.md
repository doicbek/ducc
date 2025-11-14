# Quick Test Guide for DUCC0 SHT MEX Functions

## Step 1: Build MEX Files

### On Windows:
```cmd
cd matlab\mex
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### On Linux/Mac:
```bash
cd matlab/mex
mkdir build
cd build
cmake ..
cmake --build .
```

## Step 2: Test in MATLAB

### Basic Test
```matlab
% Add paths
cd('matlab/mex');
addpath('build');
addpath('..');

% Run build test
test_build

% Run SHT tests
test_sht

% Run all tests
results = test_all
```

### Manual Test
```matlab
% Test get_gridweights
weights = ducc0.sht.get_gridweights('CC', 64);
fprintf('Weights size: %d\n', length(weights));
fprintf('All positive: %s\n', mat2str(all(weights > 0)));

% Test synthesis_2d
lmax = 32;
nalm = ((lmax+1)*(lmax+2))/2;
alm = randn(1, nalm) + 1i*randn(1, nalm);
alm(1:lmax+1) = real(alm(1:lmax+1));

map = ducc0.sht.synthesis_2d(alm, lmax, 'spin', 0, 'geometry', 'CC');
fprintf('Map size: [%s]\n', mat2str(size(map)));

% Test analysis_2d (round-trip)
alm2 = ducc0.sht.analysis_2d(map, lmax, 'spin', 0, 'geometry', 'CC');
error = max(abs(alm(:) - alm2(:)));
fprintf('Round-trip error: %e\n', error);
```

## Expected Results

### get_gridweights
- Returns array of size ntheta
- All weights are positive
- Sum of weights is reasonable

### synthesis_2d
- Returns real array
- Size: [nmaps, ntheta, nphi]
- nmaps = 1 for spin=0, nmaps = 2 for spin>0

### analysis_2d
- Returns complex array
- Size: [ncomp, nalm]
- Round-trip error < 1e-8

## Troubleshooting

### MEX Files Not Found
1. Check if MEX files were built:
   ```matlab
   dir('build/*.mex*')
   ```
2. Rebuild if needed:
   ```bash
   cd build
   cmake --build .
   ```

### Compilation Errors
1. Check MATLAB version (needs R2018b+)
2. Check compiler (needs C++17)
3. Check DUCC source files are accessible
4. Check CMake configuration

### Runtime Errors
1. Check error messages
2. Verify input array dimensions
3. Check array conversion
4. Verify DUCC source files are linked

### Round-trip Errors Too Large
1. Test with smaller lmax first
2. Check array conversion is correct
3. Verify geometry parameters
4. Test with different geometries

## Next Steps

After successful testing:
1. Fix any compilation errors
2. Fix any runtime errors
3. Optimize performance if needed
4. Add more tests for edge cases
5. Implement remaining functions

