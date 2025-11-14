# DUCC0 MATLAB MEX Interface

This document describes the MEX (MATLAB Executable) interface for DUCC0, which provides **direct access to the C++ library** without requiring Python.

## Overview

The MEX interface consists of:

1. **MEX Gateway Functions** (`matlab/mex/*_mex.cpp`): C++ functions that MATLAB can call directly
2. **Utility Functions** (`matlab/mex/ducc0_mex_utils.h`): Convert between MATLAB mxArray and DUCC ArrayDescriptor
3. **MATLAB Wrapper Functions** (`matlab/+ducc0/**/*_mex.m`): High-level MATLAB interface

## Advantages over Python Interface

- **Performance**: Direct C++ access eliminates Python overhead
- **No Dependencies**: Doesn't require Python or ducc0 Python package
- **Native Integration**: Direct memory access, no data copying overhead
- **Better Error Handling**: Direct C++ exceptions to MATLAB errors

## Building

See `matlab/mex/BUILD.md` for detailed build instructions.

Quick build in MATLAB:
```matlab
cd matlab/mex
mex -I../../src -I${matlabroot}/extern/include -cxx -std=c++17 -O3 ...
```

## Usage

After building, the MEX functions can be used directly or through wrapper functions:

```matlab
% Direct MEX call
result = ducc0_fft_mex(data, axes, forward, inorm, nthreads);

% Or through wrapper (recommended)
result = ducc0.fft.c2c_mex(data, 'axes', [0,1], 'nthreads', 4);
```

## Current Implementation Status

### Completed
- [x] MEX infrastructure (array conversion)
- [x] FFT c2c (complex-to-complex)
- [x] FFT r2c (real-to-complex)

### In Progress
- [ ] FFT c2r (complex-to-real)
- [ ] FFT r2r functions
- [ ] SHT functions
- [ ] NUFFT functions
- [ ] HEALPix functions

## Architecture

```
MATLAB Code
    ↓
MATLAB Wrapper (*_mex.m)
    ↓
MEX Gateway Function (*_mex.cpp)
    ↓
DUCC C++ Library
```

## Data Type Conversion

The MEX interface handles:
- **Real arrays**: Direct memory mapping (with dimension reversal)
- **Complex arrays**: Conversion between MATLAB's separate storage and C++ interleaved storage
- **Dimension ordering**: Automatic conversion between MATLAB (column-major) and DUCC (row-major)

## Performance Notes

- MEX interface has minimal overhead compared to Python interface
- Memory is shared when possible (no copying for simple cases)
- Multi-threading is fully supported
- For best performance, compile with optimization flags (`-O3 -march=native`)

## Migration from Python Interface

To migrate from Python to MEX interface:

1. Replace `ducc0.fft.c2c` with `ducc0.fft.c2c_mex`
2. Function signatures remain the same
3. No Python installation required
4. Rebuild MEX files for your platform

## Troubleshooting

### MEX file not found
- Ensure MEX files are in MATLAB path
- Check file extension matches your platform (`.mexw64`, `.mexa64`, `.mexmaci64`)

### Build errors
- Verify C++17 compiler is available
- Check all DUCC source files are accessible
- Ensure MATLAB include directory is correct

### Runtime errors
- Check array types and dimensions
- Verify axes indices are valid
- Ensure input arrays are not empty

## Future Work

- Complete all FFT functions
- Implement SHT, NUFFT, and HEALPix MEX functions
- Add comprehensive error handling
- Optimize complex array conversion
- Add unit tests

