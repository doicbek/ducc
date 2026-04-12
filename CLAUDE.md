# DUCC MATLAB Wrapper - Developer Guide

This repo is a fork/extension of the [DUCC](https://gitlab.mpcdf.mpg.de/mtr/ducc) (Distinctly Useful Code Collection) C++ library. The primary addition is a MATLAB MEX wrapper that exposes DUCC's algorithms (FFT, SHT, NUFFT, HEALPix) directly to MATLAB without a Python dependency.

## Repository Layout

```
ducc/
├── src/ducc0/           # Original DUCC C++ library (upstream)
│   ├── fft/             # FFT implementation
│   ├── sht/             # Spherical Harmonic Transforms
│   ├── nufft/           # Non-uniform FFTs
│   ├── healpix/         # HEALPix tessellation
│   ├── wgridder/        # Radio interferometry gridder
│   ├── infra/           # Threading, array views, error handling
│   └── math/            # Math utilities
├── matlab/              # MATLAB wrapper (custom addition)
│   ├── +ducc0/          # MATLAB package (use as ducc0.fft.c2c, etc.)
│   │   ├── +fft/        # FFT wrappers (.m files)
│   │   ├── +sht/        # SHT wrappers (.m files)
│   │   ├── +nufft/      # NUFFT wrappers (.m files)
│   │   ├── +healpix/    # HEALPix wrappers (.m files)
│   │   ├── +misc/       # Misc utilities (.m files)
│   │   └── +util/       # Array conversion helpers (.m files)
│   └── mex/             # MEX C++ gateway layer
│       ├── *.cpp        # 21 MEX gateway files (one per function)
│       ├── ducc0_mex_utils.h  # Array conversion utilities (key file)
│       └── CMakeLists.txt     # Build system
├── python/              # Python bindings (original, via nanobind)
└── CMakeLists.txt       # Main build (Python only; MEX has its own)
```

## MATLAB Wrapper Architecture

The wrapper is a three-layer stack:

```
MATLAB user code
    ↓ calls
+ducc0/*.m  (MATLAB package functions with inputParser validation)
    ↓ calls
ducc0_*_mex  (MEX gateway: mxArray ↔ DUCC C++ array conversion)
    ↓ calls
src/ducc0/*  (DUCC C++ library)
```

### Layer 1: MATLAB package (`+ducc0/`)

- Uses MATLAB's package namespace system (`+ducc0/+fft/c2c.m` → `ducc0.fft.c2c(...)`)
- Each `.m` file uses `inputParser` for named parameter handling
- Validates input types/sizes before passing to MEX
- Converts string/logical types as needed before calling MEX

### Layer 2: MEX gateways (`matlab/mex/*.cpp`)

- Each `ducc0_*_mex.cpp` is an independent MEX entry point
- Reads `mxArray*` inputs, calls DUCC C++ functions, writes output `mxArray*`
- Links against a precompiled `libducc0_static.a` for fast incremental builds

### Layer 3: Conversion utilities (`ducc0_mex_utils.h`)

The most important file in the MEX layer. Handles all MATLAB↔DUCC data format differences:

- **Column-major ↔ row-major**: MATLAB stores arrays column-major; DUCC expects row-major. Dimensions are reversed (`[rows, cols]` in MATLAB → `[cols, rows]` in DUCC).
- **Axis indexing**: MATLAB axes are 1-based column-major; DUCC axes are 0-based row-major. Conversion: `ax_ducc = ndim - 1 - (ax_matlab - 1)`.
- **Complex arrays**: MATLAB stores real and imaginary parts in separate arrays; DUCC expects interleaved `std::complex<T>`. `copyMatlabToBuffer` and `copyBufferToMatlab` handle interleaving/deinterleaving.
- The 2D and 3D cases are manually unrolled for performance; an N-dimensional fallback handles all other cases.

## Implemented Functions

All 21 MEX files and corresponding `.m` wrappers are implemented:

### `ducc0.fft`
| Function | Description |
|----------|-------------|
| `c2c` | Complex-to-complex N-dimensional FFT |
| `r2c` | Real-to-complex FFT |
| `c2r` | Complex-to-real FFT |
| `r2r_fftpack` | Real-to-real FFT (FFTPACK convention) |
| `dct` | Discrete Cosine Transform |
| `dst` | Discrete Sine Transform |
| `hartley` | Hartley transform |
| `good_size` | Find next efficient FFT size (≥ n) |

### `ducc0.sht`
| Function | Description |
|----------|-------------|
| `synthesis_2d` | alm → map on regular 2D grids (CC, GL, MW, DH, ...) |
| `analysis_2d` | map → alm on regular 2D grids |
| `adjoint_synthesis_2d` | Adjoint of synthesis_2d |
| `adjoint_analysis_2d` | Adjoint of analysis_2d |
| `synthesis` | alm → map on arbitrary rings |
| `adjoint_synthesis` | map → alm on arbitrary rings (supports sparse maps and batch mode) |
| `rotate_alm` | Rotate spherical harmonic coefficients |
| `get_gridweights` | Quadrature weights for standard grid geometries |
| `alm2map` | High-level wrapper around synthesis_2d |
| `map2alm` | High-level wrapper around analysis_2d |
| `create_alm_info` | Build alm metadata struct |
| `create_map_info` | Build map metadata struct |
| `create_sht_info` | Build combined SHT metadata struct |
| `pad_map` | Zero-pad a map in pixel space |
| `unpad_map` | Remove zero-padding from a map |
| `times_weight` | Multiply map pixels by quadrature weights |

### `ducc0.nufft`
| Function | Description |
|----------|-------------|
| `nu2u` | Non-uniform-to-uniform FFT (type 1) |
| `u2nu` | Uniform-to-non-uniform FFT (type 2) |

### `ducc0.healpix`
| Function | Description |
|----------|-------------|
| `ang2pix` | (theta, phi) → HEALPix pixel index |
| `pix2ang` | HEALPix pixel index → (theta, phi) |
| `nside2npix` | nside → total number of pixels (pure MATLAB) |
| `npix2nside` | Number of pixels → nside (pure MATLAB) |

### `ducc0.misc`
| Function | Description |
|----------|-------------|
| `vdot` | Complex inner product of two arrays |
| `l2error` | L2 error between two arrays (pure MATLAB) |

## Building

### Prerequisites

- MATLAB R2018b or later
- C++17 compiler: GCC 7+, Clang 6+, or MSVC 2019+
- CMake 3.15+

### Build steps

```bash
cd matlab/mex
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

If MATLAB is not on PATH, set:
```bash
export MATLAB_ROOT=/usr/local/MATLAB/R2024a
cmake ..
```

The build compiles all DUCC C++ sources into a static library (`libducc0_static.a`) first, then links each of the 21 MEX files against it. This avoids recompiling DUCC for every MEX file.

On Linux/macOS, a wrapper script `mex_wrapper.sh` is generated at configure time that passes `-std=c++17 -fPIC -O3 -march=native` to the MEX compiler, since MATLAB's `mex` command requires these via `CXXFLAGS=`.

### MATLAB path setup

The easiest method is the setup script:

```matlab
run('/path/to/ducc/matlab/setup_ducc0.m')
```

`setup_ducc0.m` (in `matlab/`) auto-locates `matlab/mex/build/` and adds both
paths, with a clear warning if the MEX binaries have not been built yet.  Add
the `run(...)` call to `~/Documents/MATLAB/startup.m` for a permanent setup.

Manual alternative:

```matlab
addpath('/path/to/ducc/matlab/mex/build');   % MEX binaries
addpath('/path/to/ducc/matlab');              % +ducc0 package
```

## Key Design Notes

### No OpenMP

DUCC uses its own internal thread pool (via `src/ducc0/infra/threading.cc`). OpenMP must not be used in MEX files or DUCC sources, as it conflicts with DUCC's threading model. The `nthreads` parameter passed to each function controls parallelism.

### `adjoint_synthesis` batch mode

`ducc0_sht_adjoint_synthesis_mex.cpp` (1218 lines) is the most complex MEX file. It supports:
- Dense maps: `[nmaps, npix]` or `[N, nmaps, npix]` for batch mode
- Sparse maps: MATLAB sparse matrices (`issparse()` check)
- Reuse of intermediate computations across maps in a batch for efficiency

### Sparse array handling

`adjoint_synthesis` accepts MATLAB sparse arrays directly. The MEX layer detects `mxIsSparse()` and extracts the CSC format (row indices, column pointers, values) before passing data to DUCC.

### Data type support

All MEX functions support both `double` (default) and `single` precision. The MEX functions dispatch based on `mxGetClassID()`.

## Adding a New MEX Function

1. Create `matlab/mex/ducc0_<module>_<func>_mex.cpp`:
   - Entry point: `void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])`
   - Use `ducc0_mex::copyMatlabToBuffer` / `copyBufferToMatlab` for array conversion
   - Wrap the body in a try/catch and call `ducc0_mex::handleDuccError(e)` on exception

2. Add the source file to `MEX_SOURCES` in `matlab/mex/CMakeLists.txt`

3. Create the MATLAB wrapper `matlab/+ducc0/+<module>/<func>.m`:
   - Use `inputParser` for parameter validation
   - Call the MEX function at the end

## Common Pitfalls

- **Axis confusion**: Always double-check axis conversion. MATLAB axis 1 = first dimension = columns in DUCC. A c2c FFT along MATLAB axis 1 (rows) corresponds to DUCC axis `ndim-1`.
- **Complex arrays**: Never pass a real array where a complex array is expected without explicitly casting it: `alm = complex(alm)`.
- **Ring indices in SHT**: `ringstart` in `adjoint_synthesis` is 0-based (matching DUCC's convention), not 1-based. This differs from other MATLAB indexing.
- **Column-major output**: The `.m` wrappers return MATLAB arrays in MATLAB's native column-major order. The dimension order presented to MATLAB users matches MATLAB conventions, not DUCC's internal row-major order.
