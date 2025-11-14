# DUCC0 MATLAB MEX Interface - TODO List

## Immediate Tasks (Critical for SHT Module)

### 1. Testing and Validation
- [ ] **Compile and test SHT MEX functions**
  - Build the MEX files using CMake
  - Test synthesis_2d with various geometries (CC, F1, MW, GL, DH, F2)
  - Test analysis_2d round-trip accuracy
  - Test get_gridweights for all geometries
  - Verify correct handling of 1D and 2D alm arrays
  - Test spin-0, spin-1, and spin-2 cases
  - Test with different precisions (double, single)

- [ ] **Fix any compilation errors**
  - Check for missing includes
  - Verify DUCC source files are correctly linked
  - Ensure all required DUCC functions are available

- [ ] **Validate array conversion**
  - Verify column-major to row-major conversion is correct
  - Check complex array interleaving/deinterleaving
  - Test with edge cases (small arrays, large arrays, odd dimensions)

### 2. Code Cleanup
- [ ] **Remove old/unused MEX files**
  - `ducc0_fft_mex.cpp` - appears to be old/unused
  - `ducc0_fft_r2c_mex.cpp` - incomplete, not in CMakeLists.txt
  - Either complete these files or remove them

- [ ] **Review and fix potential bugs**
  - Check for memory leaks
  - Verify error handling is complete
  - Ensure all edge cases are handled

## High Priority (Core Functionality)

### 3. Complete Remaining SHT Functions
- [ ] **adjoint_synthesis_2d**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

- [ ] **adjoint_analysis_2d**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

- [ ] **rotate_alm**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

### 4. Complete FFT Functions
- [ ] **r2c (real-to-complex)**
  - Complete or rewrite MEX function
  - Update MATLAB wrapper
  - Add tests

- [ ] **c2r (complex-to-real)**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

- [ ] **r2r_fftpack**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

## Medium Priority (Extended Functionality)

### 5. NUFFT Module
- [ ] **nu2u (non-uniform to uniform)**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

- [ ] **u2nu (uniform to non-uniform)**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

### 6. HEALPix Module
- [ ] **ang2pix (angles to pixel)**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

- [ ] **pix2ang (pixel to angles)**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

### 7. Misc Module
- [ ] **vdot (scalar product)**
  - Implement MEX function
  - Update MATLAB wrapper
  - Add tests

## Low Priority (Optimization and Polish)

### 8. Performance Optimizations
- [ ] **Optimize array conversion**
  - Reduce temporary buffer allocations
  - Use DUCC's stride support directly when possible
  - Minimize memory copies

- [ ] **Support in-place operations**
  - Add support for in-place FFT operations
  - Add support for in-place SHT operations when possible

- [ ] **Optimize multi-threading**
  - Verify thread safety
  - Optimize thread allocation
  - Add thread pool support if beneficial

### 9. Error Handling and Validation
- [ ] **Improve error messages**
  - Add more descriptive error messages
  - Include parameter values in error messages
  - Add suggestions for fixing errors

- [ ] **Add input validation**
  - Validate all input parameters
  - Check array dimensions match expectations
  - Verify parameter ranges are valid

- [ ] **Add comprehensive error handling**
  - Handle all edge cases
  - Add error recovery where possible
  - Improve error reporting

### 10. Testing and Documentation
- [ ] **Add unit tests**
  - Create test suite for all implemented functions
  - Add tests for edge cases
  - Add performance tests

- [ ] **Add integration tests**
  - Test complete workflows
  - Test with real-world data
  - Verify round-trip accuracy

- [ ] **Update documentation**
  - Add more examples
  - Add troubleshooting guide
  - Add performance tuning guide
  - Add API reference documentation

### 11. Build System Improvements
- [ ] **Improve CMake configuration**
  - Add support for different MATLAB versions
  - Add support for different compilers
  - Add better error messages for missing dependencies

- [ ] **Add build scripts**
  - Add Windows build script
  - Add Linux build script
  - Add macOS build script

- [ ] **Add CI/CD support**
  - Add GitHub Actions workflow
  - Add automated testing
  - Add automated building

## Known Issues

### Array Conversion
- Current implementation uses temporary buffers which adds overhead
- May be slow for very large arrays
- Could be optimized using DUCC's stride support

### Type Support
- Currently supports double and single precision only
- Could add support for integer types if needed

### Error Handling
- Some edge cases may not be fully handled
- Error messages could be more descriptive

## Future Enhancements

1. **GPU Support**: Add support for GPU-accelerated operations
2. **Distributed Computing**: Add support for distributed computing
3. **Additional FFT Functions**: Add DCT, DST, Hartley transforms
4. **Additional SHT Functions**: Add more SHT variants
5. **Performance Profiling**: Add performance profiling tools
6. **Memory Profiling**: Add memory profiling tools

## Notes

- Priority is on testing and validating the current SHT implementation
- Focus on core functionality before adding new features
- Performance optimizations can be done incrementally
- Documentation should be updated as features are added

