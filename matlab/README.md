# DUCC0 MATLAB Interface

MATLAB MEX wrapper for [DUCC](https://gitlab.mpcdf.mpg.de/mtr/ducc) (Distinctly Useful Code Collection), providing direct C++ access to high-performance FFT, spherical harmonic transform, and non-uniform FFT algorithms without a Python dependency.

## Requirements

- MATLAB R2018b or later
- C++17 compiler (GCC 7+, Clang 6+, or MSVC 2019+)
- CMake 3.15 or later

## Building

```bash
cd matlab/mex
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

If MATLAB is not on PATH, set `MATLAB_ROOT` first:
```bash
export MATLAB_ROOT=/usr/local/MATLAB/R2024a
```

See `mex/INSTALL.md` for detailed build instructions and troubleshooting.

## Setup

The easiest way is to run the provided setup script:

```matlab
run('/path/to/ducc/matlab/setup_ducc0.m')
```

For a permanent setup add that line to `~/Documents/MATLAB/startup.m`.

Alternatively, set the paths manually:

```matlab
addpath('/path/to/ducc/matlab/mex/build');   % compiled MEX binaries
addpath('/path/to/ducc/matlab');              % +ducc0 package
```

## Quick Examples

### FFT

```matlab
% Complex-to-complex 2D FFT
x = randn(128, 128) + 1i*randn(128, 128);
y = ducc0.fft.c2c(x);
% Inverse FFT (inorm=2 divides by N)
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);

% Real-to-complex FFT
x = randn(256, 256);
y = ducc0.fft.r2c(x);

% Efficient FFT size
n_good = ducc0.fft.good_size(1000);  % → 1024
```

### Spherical Harmonic Transforms

```matlab
lmax = 64;
nalm = (lmax+1)*(lmax+2)/2;
alm = randn(1, nalm) + 1i*randn(1, nalm);

% Synthesis: alm → map (Clenshaw-Curtis grid)
map = ducc0.sht.synthesis_2d(alm, lmax, 'geometry', 'CC');

% Analysis: map → alm
alm2 = ducc0.sht.analysis_2d(map, lmax, 'geometry', 'CC');

% Quadrature weights
w = ducc0.sht.get_gridweights('CC', lmax+1);
```

### Non-uniform FFT

```matlab
% Type 1: non-uniform points → uniform grid
npts  = 1000;
coord = rand(npts, 2) * 2*pi;        % [npts, ndim]
amp   = randn(npts, 1) + 1i*randn(npts, 1);
grid  = ducc0.nufft.nu2u(amp, coord, 'grid_shape', [64, 64]);

% Type 2: uniform grid → non-uniform points
result = ducc0.nufft.u2nu(grid, coord);
```

### HEALPix

```matlab
nside = 64;
npix = ducc0.healpix.nside2npix(nside);  % pure MATLAB

theta = pi/4;
phi = pi/3;
pix = ducc0.healpix.ang2pix(nside, theta, phi);
[t, p] = ducc0.healpix.pix2ang(nside, pix);
```

## Module Reference

### `ducc0.fft`

| Function | Description |
|----------|-------------|
| `c2c(a, ...)` | Complex-to-complex N-dim FFT |
| `r2c(a, ...)` | Real-to-complex FFT |
| `c2r(a, ...)` | Complex-to-real FFT |
| `r2r_fftpack(a, ...)` | Real-to-real FFT (FFTPACK convention) |
| `dct(a, ...)` | Discrete Cosine Transform |
| `dst(a, ...)` | Discrete Sine Transform |
| `hartley(a, ...)` | Hartley transform |
| `good_size(n)` | Next efficient FFT size ≥ n |

Common parameters: `axes` (1-based), `forward` (bool), `inorm` (0/1/2), `nthreads`.

**Normalization** (`inorm`): `0` = none, `1` = divide by √N, `2` = divide by N.

### `ducc0.sht`

| Function | Description |
|----------|-------------|
| `synthesis_2d(alm, lmax, ...)` | alm → map on regular 2D grids |
| `analysis_2d(map, lmax, ...)` | map → alm on regular 2D grids |
| `adjoint_synthesis_2d(map, lmax, ...)` | Adjoint of synthesis_2d |
| `adjoint_analysis_2d(alm, lmax, ...)` | Adjoint of analysis_2d |
| `synthesis(alm, lmax, spin, theta, nphi, phi0, ringstart, ...)` | alm → map, arbitrary rings |
| `adjoint_synthesis(map, lmax, spin, theta, nphi, phi0, ringstart, ...)` | map → alm, arbitrary rings |
| `rotate_alm(alm, lmax, psi, theta, phi, ...)` | Rotate alm coefficients |
| `get_gridweights(geometry, ntheta)` | Quadrature weights |
| `alm2map(alm, lmax, ...)` | High-level synthesis wrapper |
| `map2alm(map, lmax, ...)` | High-level analysis wrapper |

**Supported geometries**: `'CC'` (Clenshaw-Curtis), `'GL'` (Gauss-Legendre), `'MW'`, `'MWflip'`, `'DH'` (Driscoll-Healy), `'F1'`, `'F2'`.

`adjoint_synthesis` additionally supports:
- Sparse MATLAB arrays (detected via `issparse()`)
- Batch mode: 3D input `[N, nmaps, npix]` with `N_batch` parameter

### `ducc0.nufft`

| Function | Description |
|----------|-------------|
| `nu2u(points, coord, shape, ...)` | Non-uniform → uniform (type 1) |
| `u2nu(grid, coord, ...)` | Uniform → non-uniform (type 2) |

Parameters: `epsilon` (accuracy), `nthreads`, `verbosity`.

### `ducc0.healpix`

| Function | Description |
|----------|-------------|
| `ang2pix(nside, theta, phi, ...)` | Angles → pixel index (MEX) |
| `pix2ang(nside, pix, ...)` | Pixel index → angles (MEX) |
| `nside2npix(nside)` | nside → total pixels (MATLAB) |
| `npix2nside(npix)` | Total pixels → nside (MATLAB) |

### `ducc0.misc`

| Function | Description |
|----------|-------------|
| `vdot(a, b)` | Complex inner product (MEX) |
| `l2error(a, b)` | Relative L2 error (MATLAB) |

## Parameter Conventions

- **Axes**: 1-based MATLAB indexing (axis 1 = rows, axis 2 = columns)
- **Threading**: `nthreads=0` uses all available cores; `nthreads=N` uses N threads
- **Precision**: pass `single` arrays for single precision, `double` for double precision

## Implementation Notes

MATLAB stores arrays in column-major order with separate real/imaginary arrays for complex data. DUCC uses row-major order with interleaved complex. The MEX layer handles all conversions automatically via `ducc0_mex_utils.h`.

See `CLAUDE.md` in the repository root for developer documentation, including how to add new MEX functions.

## License

Same as DUCC: GNU General Public License v2 or later.

## References

- DUCC source: https://gitlab.mpcdf.mpg.de/mtr/ducc
- DUCC Python docs: https://mtr.pages.mpcdf.de/ducc
