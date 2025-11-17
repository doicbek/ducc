# DUCC0 MATLAB MEX Interface - Testing Summary

## Overview

This document provides a summary of the testing framework for the DUCC0 MATLAB MEX interface, specifically focusing on the SHT (Spherical Harmonic Transform) module.

## Test Scripts

### 1. test_build.m
**Purpose:** Verify that MEX files are built and available.

**What it tests:**
- Checks if MEX files exist in the build directory
- Tries to call MEX functions directly
- Checks if MATLAB wrapper functions exist

**Usage:**
```matlab
test_build
```

### 2. test_simple.m
**Purpose:** Basic tests to verify the implementation works.

**What it tests:**
1. Check if MEX files exist
2. Test `get_gridweights`
3. Test `synthesis_2d` (basic)
4. Test `analysis_2d` (round-trip)
5. Test different geometries (CC, F1, MW, GL, DH)

**Usage:**
```matlab
test_simple
```

### 3. test_sht.m
**Purpose:** Comprehensive SHT function tests.

**What it tests:**
1. `get_gridweights` - Quadrature weights
2. `synthesis_2d` - Spherical harmonic synthesis (1D alm)
3. `analysis_2d` - Spherical harmonic analysis (round-trip)
4. `synthesis_2d` - Spherical harmonic synthesis (2D alm)
5. Different geometries (CC, F1, MW, GL, DH)
6. Spin-2 fields
7. Single precision
8. Edge cases (small lmax, different mmax)

**Usage:**
```matlab
test_sht
```

### 4. test_all.m
**Purpose:** Comprehensive test suite for all implemented functions.

**What it tests:**
- MEX file availability
- FFT functions (c2c, good_size)
- SHT functions (synthesis_2d, analysis_2d, get_gridweights)
- HEALPix functions (nside2npix, npix2nside)
- Misc functions (l2error)

**Usage:**
```matlab
results = test_all
```

**Returns:**
- `results` structure with test results and errors

## Build Scripts

### 1. build_test.bat (Windows)
**Purpose:** Automated build test for Windows.

**Usage:**
```cmd
build_test.bat
```

### 2. build_test.sh (Linux/Mac)
**Purpose:** Automated build test for Linux/Mac.

**Usage:**
```bash
chmod +x build_test.sh
./build_test.sh
```

## Expected Results

### get_gridweights
- Returns array of size ntheta
- All weights are positive
- Sum of weights is reasonable
- Weights are valid for the given geometry

### synthesis_2d
- Returns real array
- Size: [nmaps, ntheta, nphi]
- nmaps = 1 for spin=0, nmaps = 2 for spin>0
- Map values are reasonable
- Works with 1D and 2D alm arrays
- Works with different geometries
- Works with single and double precision

### analysis_2d
- Returns complex array
- Size: [ncomp, nalm]
- Round-trip error < 1e-8 (numerical precision)
- Works with different geometries
- Works with single and double precision

### Round-trip Test
For synthesis_2d → analysis_2d:
- Round-trip error should be < 1e-8 (numerical precision)
- Error depends on lmax, geometry, and numerical precision
- Error may be larger for single precision

## Test Parameters

### Default Test Parameters
- `lmax = 32` - Maximum multipole order
- `mmax = lmax` - Maximum m order
- `spin = 0` - Spin value (0, 1, or 2)
- `geometry = 'CC'` - Grid geometry
- `ntheta = lmax + 2` - Number of theta rings (CC)
- `nphi = 2 * mmax + 2` - Number of phi pixels per ring

### Geometry-specific Parameters
- **CC**: `ntheta = lmax + 2`
- **F1**: `ntheta = lmax + 1`
- **MW**: `ntheta = lmax + 1`
- **GL**: `ntheta = lmax + 1`
- **DH**: `ntheta = 2 * lmax + 2`
- **F2**: `ntheta = lmax + 1`

## Troubleshooting

### MEX Files Not Found
**Problem:** Test scripts report MEX files not found.

**Solution:**
1. Check if MEX files were built:
   ```matlab
   dir('build/*.mex*')
   ```
2. Rebuild if needed:
   ```bash
   cd build
   cmake --build .
   ```
3. Check MATLAB path:
   ```matlab
   addpath('build');
   ```

### Compilation Errors
**Problem:** MEX files fail to compile.

**Common issues:**
1. **Missing MATLAB:** Ensure MATLAB is installed and in PATH
2. **Missing compiler:** Install Visual Studio (Windows) or GCC/Clang (Linux/Mac)
3. **Missing DUCC source:** Ensure DUCC source files are accessible
4. **Wrong MATLAB version:** Ensure MATLAB R2018b or later

**Solution:**
1. Check MATLAB installation:
   ```matlab
   mex -setup
   ```
2. Check compiler:
   ```bash
   gcc --version  # Linux/Mac
   cl            # Windows (Visual Studio)
   ```
3. Check DUCC source:
   ```bash
   ls ../../src/ducc0/sht/sht.h
   ```

### Runtime Errors
**Problem:** MEX functions fail at runtime.

**Common issues:**
1. **Missing dependencies:** DUCC source files not linked
2. **Wrong array dimensions:** Input arrays have wrong shape
3. **Memory errors:** Array conversion issues

**Solution:**
1. Check error messages - they should indicate the problem
2. Verify input array dimensions
3. Check array conversion in MEX functions
4. Verify DUCC source files are accessible

### Round-trip Errors Too Large
**Problem:** Round-trip error is > 1e-8.

**Possible causes:**
1. **Numerical precision:** Normal for very large lmax
2. **Array conversion:** Column-major to row-major conversion issues
3. **Geometry issues:** Some geometries may have higher errors
4. **Implementation bugs:** Check synthesis_2d and analysis_2d

**Solution:**
1. Test with smaller lmax first
2. Verify array conversion is correct
3. Test with different geometries
4. Check for implementation bugs

## Performance Testing

### Benchmark Script
```matlab
% Benchmark SHT functions
lmax = 64;
nalm = ((lmax+1)*(lmax+2))/2;
alm = randn(1, nalm) + 1i*randn(1, nalm);
alm(1:lmax+1) = real(alm(1:lmax+1));

% Synthesis
tic;
map = ducc0.sht.synthesis_2d(alm, lmax, 'spin', 0, 'geometry', 'CC');
t_synth = toc;

% Analysis
tic;
alm2 = ducc0.sht.analysis_2d(map, lmax, 'spin', 0, 'geometry', 'CC');
t_anal = toc;

fprintf('Synthesis time: %f s\n', t_synth);
fprintf('Analysis time: %f s\n', t_anal);
```

### Expected Performance
- Synthesis: ~0.1-1 s for lmax=64 (depends on hardware)
- Analysis: ~0.1-1 s for lmax=64 (depends on hardware)
- Multi-threading: Should scale with number of threads

## Next Steps

After testing:
1. **Fix any compilation errors** - Essential for the code to work
2. **Fix any runtime errors** - Verify implementation is correct
3. **Optimize performance** - Improve array conversion if needed
4. **Add more tests** - Test edge cases and error conditions
5. **Update documentation** - Document any issues found

## See Also

- `TEST_GUIDE.md` - Comprehensive testing guide
- `QUICK_TEST.md` - Quick start guide
- `INSTALL.md` - Installation instructions
- `README.md` - Usage documentation
- `SUMMARY.md` - Implementation status
- `TODO.md` - Pending tasks



