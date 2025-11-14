# Next Steps for DUCC0 MATLAB MEX Interface

## Critical Tasks (Do First)

### 1. Test and Validate Current SHT Implementation

**Priority: CRITICAL**

The SHT module has been implemented but needs testing:

1. **Build the MEX files**
   ```bash
   cd matlab/mex
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

2. **Test synthesis_2d**
   ```matlab
   lmax = 32;
   nalm = ((lmax+1)*(lmax+2))/2;
   alm = randn(1, nalm) + 1i*randn(1, nalm);
   map = ducc0.sht.synthesis_2d(alm, lmax, 'spin', 0, 'geometry', 'CC');
   ```

3. **Test analysis_2d (round-trip)**
   ```matlab
   alm2 = ducc0.sht.analysis_2d(map, lmax, 'spin', 0, 'geometry', 'CC');
   error = max(abs(alm(:) - alm2(:)));
   fprintf('Round-trip error: %e\n', error);
   ```

4. **Test get_gridweights**
   ```matlab
   weights = ducc0.sht.get_gridweights('CC', 64);
   assert(all(weights > 0), 'weights should be positive');
   ```

5. **Test different geometries**
   - Test all geometries: CC, F1, MW, MWflip, GL, DH, F2
   - Test spin-0, spin-1, spin-2
   - Test 1D and 2D alm arrays
   - Test double and single precision

### 2. Fix Any Compilation Errors

**Priority: CRITICAL**

If there are compilation errors:
1. Check for missing includes
2. Verify DUCC source files are correctly linked
3. Check for missing DUCC functions
4. Verify all required headers are included

### 3. Clean Up Old/Unused Files

**Priority: HIGH**

Remove or complete old MEX files:
- `ducc0_fft_mex.cpp` - appears to be old/unused
- `ducc0_fft_r2c_mex.cpp` - incomplete, not in CMakeLists.txt

**Action**: Either complete these files or remove them from the repository.

## High Priority Tasks

### 4. Complete Remaining SHT Functions

**Priority: HIGH**

Implement the remaining SHT functions:
- `adjoint_synthesis_2d` - Adjoint synthesis
- `adjoint_analysis_2d` - Adjoint analysis  
- `rotate_alm` - Rotate spherical harmonic coefficients

These are important for completeness of the SHT module.

### 5. Complete FFT Functions

**Priority: HIGH**

Complete the FFT module:
- `r2c` - Real-to-complex FFT (partially implemented)
- `c2r` - Complex-to-real FFT
- `r2r_fftpack` - Real-to-real FFT

### 6. Validate Array Conversion

**Priority: HIGH**

Test and fix array conversion:
- Verify column-major to row-major conversion is correct
- Test with various array sizes and shapes
- Verify complex array interleaving/deinterleaving
- Check edge cases (small arrays, large arrays, odd dimensions)

## Medium Priority Tasks

### 7. Implement NUFFT Module

**Priority: MEDIUM**

Implement NUFFT functions:
- `nu2u` - Non-uniform to uniform FFT
- `u2nu` - Uniform to non-uniform FFT

### 8. Implement HEALPix MEX Functions

**Priority: MEDIUM**

Complete HEALPix module:
- `ang2pix` - Convert angles to pixel indices (MEX)
- `pix2ang` - Convert pixel indices to angles (MEX)

Note: `nside2npix` and `npix2nside` are already implemented in MATLAB.

### 9. Implement Misc Functions

**Priority: MEDIUM**

Complete misc module:
- `vdot` - Scalar product (MEX)

Note: `l2error` is already implemented in MATLAB.

## Low Priority Tasks (Optimization)

### 10. Performance Optimizations

**Priority: LOW**

- Optimize array conversion to reduce temporary buffers
- Use DUCC's stride support directly when possible
- Support in-place operations where possible
- Optimize multi-threading

### 11. Error Handling and Validation

**Priority: LOW**

- Improve error messages
- Add comprehensive input validation
- Handle all edge cases
- Add better error recovery

### 12. Documentation and Testing

**Priority: LOW**

- Add unit tests
- Add integration tests
- Add more examples
- Add troubleshooting guide
- Add performance tuning guide

## Immediate Action Items

1. **Build and test the SHT MEX functions** - This is the most critical task
2. **Fix any compilation errors** - Essential for the code to work
3. **Clean up old/unused files** - Keep the codebase clean
4. **Test array conversion** - Verify correctness of the implementation
5. **Validate round-trip accuracy** - Ensure synthesis/analysis are correct

## Testing Checklist

- [ ] Build MEX files successfully
- [ ] Test synthesis_2d with all geometries
- [ ] Test analysis_2d with all geometries
- [ ] Test round-trip accuracy (synthesis → analysis)
- [ ] Test get_gridweights for all geometries
- [ ] Test spin-0, spin-1, spin-2 cases
- [ ] Test 1D and 2D alm arrays
- [ ] Test double and single precision
- [ ] Test with various array sizes
- [ ] Test edge cases (small arrays, large arrays)
- [ ] Verify error handling
- [ ] Check memory usage
- [ ] Verify multi-threading works correctly

## Known Issues

1. **Old/unused MEX files** - `ducc0_fft_mex.cpp` and `ducc0_fft_r2c_mex.cpp` need to be cleaned up
2. **Array conversion** - Needs thorough testing to ensure correctness
3. **Error handling** - Some edge cases may not be fully handled
4. **Performance** - Array conversion may add overhead for large arrays

## Next Session Goals

1. Build and test the SHT MEX functions
2. Fix any compilation errors
3. Clean up old/unused files
4. Test array conversion thoroughly
5. Validate round-trip accuracy

## Notes

- The SHT module implementation is complete but needs testing
- Focus on testing and validation before adding new features
- Performance optimizations can be done incrementally
- Documentation should be updated as features are tested and validated

