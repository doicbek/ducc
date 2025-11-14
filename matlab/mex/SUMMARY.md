# DUCC0 MATLAB MEX Interface - Summary

## Overview

This MATLAB MEX interface provides direct access to the DUCC C++ library from MATLAB, without requiring Python. It mirrors the Python API structure and provides MATLAB-friendly parameter handling.

## Architecture

### Components

1. **MEX Gateway Functions** (`*_mex.cpp`):
   - Entry points called by MATLAB
   - Handle MATLAB mxArray conversion
   - Call DUCC C++ functions
   - Return results as MATLAB arrays

2. **MEX Utilities** (`ducc0_mex_utils.h`):
   - Convert between MATLAB mxArray and DUCC array views
   - Handle column-major to row-major conversion
   - Handle complex array interleaving/deinterleaving
   - Convert axes from MATLAB 1-based to DUCC 0-based

3. **MATLAB Wrapper Functions** (`+ducc0/*.m`):
   - High-level MATLAB interface
   - Handle MATLAB-style parameter parsing
   - Call MEX functions
   - Provide MATLAB-friendly error messages

## Implementation Status

### Completed

- [x] MEX infrastructure (array conversion utilities)
- [x] FFT c2c MEX function (complex-to-complex FFT)
- [x] FFT good_size MEX function (find efficient FFT size)
- [x] MATLAB wrapper functions for FFT module
- [x] Comprehensive documentation (README, INSTALL)
- [x] CMake build system
- [x] Array conversion (column-major to row-major)
- [x] Complex array handling (interleaving/deinterleaving)
- [x] Axis conversion (1-based to 0-based, column-major to row-major)

### In Progress

- [ ] FFT r2c MEX function (real-to-complex FFT)
- [ ] FFT c2r MEX function (complex-to-real FFT)
- [ ] Complete error handling and validation
- [ ] Performance optimizations

### Planned

- [ ] SHT MEX functions (synthesis_2d, analysis_2d, rotate_alm, get_gridweights)
- [ ] NUFFT MEX functions (nu2u, u2nu)
- [ ] HEALPix MEX functions (nside2npix, npix2nside, ang2pix, pix2ang)
- [ ] Misc MEX functions (vdot, l2error)
- [ ] Additional FFT functions (r2r_fftpack, dct, dst, hartley)
- [ ] Unit tests and validation
- [ ] Performance benchmarks

## Key Features

### Array Conversion

- **Dimension Reordering**: Converts MATLAB's column-major dimensions to DUCC's row-major order
- **Complex Array Handling**: Interleaves/deinterleaves real and imaginary parts
- **Memory Management**: Uses temporary buffers for conversion (may be optimized in future)

### Axis Conversion

- **1-based to 0-based**: Subtracts 1 from MATLAB axis indices
- **Column-major to Row-major**: Reverses axis order to match DUCC's expectations

### Error Handling

- **MATLAB-friendly Errors**: Converts C++ exceptions to MATLAB errors
- **Clear Error Messages**: Provides descriptive error messages
- **Input Validation**: Validates inputs before processing

## Usage

### Basic Usage

```matlab
% Add paths
addpath('/path/to/ducc/matlab/mex/build');
addpath('/path/to/ducc/matlab');

% FFT example
x = randn(128, 128) + 1i*randn(128, 128);
y = ducc0.fft.c2c(x);
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);

% Good size
n = 1000;
n_good = ducc0.fft.good_size(n);
```

### Advanced Usage

```matlab
% Transform specific axes
x = randn(64, 64, 64);
y = ducc0.fft.c2c(x, 'axes', [1, 2]);

% Use multiple threads
y = ducc0.fft.c2c(x, 'nthreads', 8);

% Custom normalization
y = ducc0.fft.c2c(x, 'inorm', 1);  % Divide by sqrt(N)
```

## Performance Considerations

### Array Conversion Overhead

- For large arrays, the column-major to row-major conversion adds overhead
- This may be optimized in future versions by using DUCC's stride support directly
- Current implementation uses temporary buffers for correctness

### Multi-threading

- Use the `nthreads` parameter to control parallelism
- Default (0) uses system default
- For large arrays, multi-threading can significantly improve performance

### Memory Usage

- Temporary buffers are used for array conversion
- This may increase memory usage for large arrays
- Future versions may support in-place operations when possible

## Known Limitations

1. **Array Conversion**: Current implementation uses temporary buffers, which adds overhead for large arrays
2. **In-place Operations**: Not yet supported (always creates new arrays)
3. **Type Support**: Currently supports double and single precision only
4. **Error Handling**: Some edge cases may not be fully handled
5. **Performance**: Array conversion may be a bottleneck for very large arrays

## Future Improvements

1. **Optimize Array Conversion**: Use DUCC's stride support directly to avoid copying
2. **In-place Operations**: Support in-place operations when possible
3. **Better Error Messages**: Provide more descriptive error messages
4. **Unit Tests**: Add comprehensive unit tests
5. **Performance Benchmarks**: Compare performance with Python interface
6. **Additional Functions**: Implement remaining DUCC functions
7. **Documentation**: Add more examples and use cases

## Testing

### Basic Tests

```matlab
% Test FFT round-trip
x = randn(64, 64) + 1i*randn(64, 64);
y = ducc0.fft.c2c(x);
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);
fprintf('Error: %e\n', max(abs(x(:) - z(:)))));

% Test good_size
n = 1000;
n_good = ducc0.fft.good_size(n);
assert(n_good >= n, 'good_size should return value >= input');
```

### Expected Results

- FFT round-trip error should be < 1e-10 (numerical precision)
- good_size should return value >= input value
- All functions should handle edge cases correctly

## Documentation

- **README.md**: Overview and usage
- **INSTALL.md**: Installation instructions
- **SUMMARY.md**: This file
- **API Documentation**: See MATLAB help for individual functions

## Support

- Check README.md for common issues
- Report issues on the DUCC GitLab repository
- Consult DUCC documentation: https://mtr.pages.mpcdf.de/ducc

## License

Same as DUCC (GPL v2 or later, with some files also under BSD 3-clause).

