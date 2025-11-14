# DUCC0 MATLAB Examples

This document provides practical examples for using the DUCC0 MATLAB wrapper.

## Table of Contents

1. [Fast Fourier Transforms](#fast-fourier-transforms)
2. [Spherical Harmonic Transforms](#spherical-harmonic-transforms)
3. [Non-uniform FFTs](#non-uniform-ffts)
4. [HEALPix Operations](#healpix-operations)
5. [Utility Functions](#utility-functions)

## Fast Fourier Transforms

### Basic Complex FFT

```matlab
% Create test data
x = randn(128, 128) + 1i*randn(128, 128);

% Forward FFT
y = ducc0.fft.c2c(x);

% Inverse FFT (with normalization)
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);

% Check reconstruction
fprintf('Reconstruction error: %e\n', max(abs(x(:) - z(:))));
```

### Real-to-Complex FFT

```matlab
% Real input data
x = randn(256, 256);

% Forward transform (MEX not yet implemented, placeholder)
% y = ducc0.fft.r2c(x, 'axes', [1, 2]);
% 
% % Note: output size is reduced along last axis
% fprintf('Input size: [%d, %d]\n', size(x));
% fprintf('Output size: [%d, %d]\n', size(y));
```

### Multi-threaded FFT

```matlab
% Large array
x = randn(1024, 1024, 64) + 1i*randn(1024, 1024, 64);

% Use 8 threads
tic;
y = ducc0.fft.c2c(x, 'nthreads', 8);
toc;
```

### Finding Good FFT Size

```matlab
% Find efficient size for FFT
n = 1000;
n_good = ducc0.fft.good_size(n);
fprintf('Original size: %d, Good size: %d\n', n, n_good);

% For real FFTs
n_good_real = ducc0.fft.good_size(n, 'real', true);
fprintf('Good size for real FFT: %d\n', n_good_real);
```

## Spherical Harmonic Transforms

### Basic Synthesis and Analysis

```matlab
% Set parameters
lmax = 32;
mmax = 32;
ntheta = lmax + 1;
nphi = 2 * mmax + 2;

% Generate random spherical harmonic coefficients
nalm = (lmax+1)*(lmax+2)/2 + (lmax+1)*(mmax-lmax);
alm = randn(1, nalm) + 1i*randn(1, nalm);
% Make m=0 modes real
alm(1:lmax+1) = real(alm(1:lmax+1));

% Synthesize map
map = ducc0.sht.synthesis_2d(alm, lmax, ...
    'spin', 0, ...
    'mmax', mmax, ...
    'ntheta', ntheta, ...
    'nphi', nphi, ...
    'geometry', 'CC');

% Analyze back
alm2 = ducc0.sht.analysis_2d(map, lmax, ...
    'spin', 0, ...
    'mmax', mmax, ...
    'geometry', 'CC');

% Check round-trip error
error = max(abs(alm(:) - alm2(:)));
fprintf('Round-trip error: %e\n', error);
```

### Different Grid Geometries

```matlab
lmax = 64;
geometries = {'CC', 'F1', 'MW', 'GL', 'DH'};

for i = 1:length(geometries)
    geo = geometries{i};
    
    % Get appropriate number of rings
    if strcmp(geo, 'CC')
        ntheta = lmax + 2;
    elseif strcmp(geo, 'DH')
        ntheta = 2*lmax + 2;
    else
        ntheta = lmax + 1;
    end
    
    % Generate test data
    nalm = (lmax+1)*(lmax+2)/2;
    alm = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));
    
    % Synthesize
    map = ducc0.sht.synthesis_2d(alm, lmax, ...
        'spin', 0, ...
        'ntheta', ntheta, ...
        'nphi', 2*lmax+2, ...
        'geometry', geo);
    
    fprintf('Geometry %s: map size [%d, %d, %d]\n', geo, size(map));
end
```

### Spin-2 Fields

```matlab
lmax = 32;
spin = 2;
ntheta = lmax + 1;
nphi = 2*lmax + 2;

% Spin-2 fields have 2 components (E and B modes)
nalm = (lmax+1)*(lmax+2)/2;
alm = randn(2, nalm) + 1i*randn(2, nalm);
alm(:, 1:lmax+1) = real(alm(:, 1:lmax+1));

% Synthesize
map = ducc0.sht.synthesis_2d(alm, lmax, ...
    'spin', spin, ...
    'ntheta', ntheta, ...
    'nphi', nphi);

% Map has 2 components
fprintf('Map shape: [%d, %d, %d]\n', size(map));
```

### Getting Quadrature Weights

```matlab
% Get weights for Clenshaw-Curtis grid
ntheta = 65;
weights = ducc0.sht.get_gridweights('CC', ntheta);

% Weights need to be divided by number of pixels per ring
nphi = 128;
weights_per_pixel = weights / nphi;

fprintf('Total weight: %f (should be ~4*pi = %f)\n', ...
    sum(weights_per_pixel) * nphi, 4*pi);
```

## Non-uniform FFTs

### Uniform to Non-uniform FFT (u2nu)

```matlab
% Generate uniform grid
grid_size = 256;
grid = randn(grid_size, 1) + 1i*randn(grid_size, 1);

% Generate random non-uniform coordinates
npoints = 1000;
coord = (rand(npoints, 1) - 0.5) * 2*pi;  % Random coordinates in [-pi, pi]

% Transform to non-uniform points
points = ducc0.nufft.u2nu(grid, coord, ...
    'epsilon', 1e-12, ...
    'periodicity', 2*pi, ...
    'nthreads', 4);

fprintf('Points shape: [%d]\n', length(points));
```

### Non-uniform to Uniform FFT (nu2u)

```matlab
% Note: nu2u MEX function is not yet fully implemented
% This example shows the expected usage once implemented

% Generate random non-uniform points
npoints = 1000;
points = randn(npoints, 1) + 1i*randn(npoints, 1);
coord = (rand(npoints, 1) - 0.5) * 2*pi;  % Random coordinates in [-pi, pi]

% Transform to uniform grid (when implemented)
% grid_size = 256;
% grid = ducc0.nufft.nu2u(points, coord, ...
%     'grid_shape', [grid_size], ...
%     'epsilon', 1e-12, ...
%     'periodicity', 2*pi, ...
%     'nthreads', 4);
```

## HEALPix Operations

### Basic Pixel Operations

```matlab
% Set nside
nside = 256;
npix = ducc0.healpix.nside2npix(nside);
fprintf('Nside: %d, Number of pixels: %d\n', nside, npix);

% Convert angles to pixels
theta = pi/3;  % Colatitude
phi = pi/4;    % Azimuth
pix = ducc0.healpix.ang2pix(nside, theta, phi);

% Convert back
[theta2, phi2] = ducc0.healpix.pix2ang(nside, pix);
fprintf('Original: theta=%.6f, phi=%.6f\n', theta, phi);
fprintf('Recovered: theta=%.6f, phi=%.6f\n', theta2, phi2);
```

### Array Operations

```matlab
nside = 128;

% Generate random angles
n = 1000;
theta = rand(n, 1) * pi;  % Colatitude in [0, pi]
phi = rand(n, 1) * 2*pi;  % Azimuth in [0, 2*pi]

% Convert to pixels
pix = ducc0.healpix.ang2pix(nside, theta, phi);

% Convert back
[theta2, phi2] = ducc0.healpix.pix2ang(nside, pix);

% Check error
theta_error = max(abs(theta - theta2));
phi_error = max(abs(phi - phi2));
fprintf('Max theta error: %e\n', theta_error);
fprintf('Max phi error: %e\n', phi_error);
```

### NESTED vs RING Ordering

```matlab
nside = 64;
theta = pi/3;
phi = pi/4;

% RING ordering (default)
pix_ring = ducc0.healpix.ang2pix(nside, theta, phi, 'nest', false);

% NESTED ordering
pix_nest = ducc0.healpix.ang2pix(nside, theta, phi, 'nest', true);

fprintf('RING pixel: %d\n', pix_ring);
fprintf('NESTED pixel: %d\n', pix_nest);
```

## Utility Functions

### Scalar Product

```matlab
% Real arrays
a = randn(100, 100);
b = randn(100, 100);
dot_product = ducc0.misc.vdot(a, b);
fprintf('Scalar product: %f\n', dot_product);

% Complex arrays
a = randn(100) + 1i*randn(100);
b = randn(100) + 1i*randn(100);
dot_product = ducc0.misc.vdot(a, b);
fprintf('Complex scalar product: %f%+.6fi\n', real(dot_product), imag(dot_product));
```

### L2 Error

```matlab
% Compare two arrays
a = randn(128, 128);
b = a + 1e-10 * randn(128, 128);  % Add small noise

% Compute L2 error manually
error = sqrt(sum(abs(a(:) - b(:)).^2));
fprintf('L2 error: %e\n', error);
```

## Performance Tips

1. **Use multi-threading** for large arrays:
   ```matlab
   result = ducc0.fft.c2c(x, 'nthreads', 8);
   ```

2. **Pre-allocate output arrays** when possible:
   ```matlab
   out = zeros(size(x), 'like', 1+1i);
   y = ducc0.fft.c2c(x, 'out', out);
   ```

3. **Choose appropriate normalization**:
   ```matlab
   % For forward then inverse, use inorm=2 on inverse
   y = ducc0.fft.c2c(x, 'forward', true, 'inorm', 0);
   z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);
   ```

4. **Use good FFT sizes**:
   ```matlab
   n = 1000;
   n_good = ducc0.fft.good_size(n);
   x_padded = zeros(n_good, 1);
   x_padded(1:n) = x;
   ```

## Common Patterns

### Round-trip Test

```matlab
function test_roundtrip(func_forward, func_inverse, data, varargin)
    % Test that forward then inverse gives original data
    forward_result = func_forward(data, varargin{:});
    inverse_result = func_inverse(forward_result, varargin{:});
    error = max(abs(data(:) - inverse_result(:)));
    fprintf('Round-trip error: %e\n', error);
end

% Example usage
x = randn(128, 128) + 1i*randn(128, 128);
test_roundtrip(@(x) ducc0.fft.c2c(x, 'forward', true), ...
               @(x) ducc0.fft.c2c(x, 'forward', false, 'inorm', 2), ...
               x);
```

### Batch Processing

```matlab
% Process multiple arrays
data_cell = {randn(64, 64), randn(64, 64), randn(64, 64)};
results = cell(size(data_cell));

for i = 1:length(data_cell)
    results{i} = ducc0.fft.c2c(data_cell{i}, 'nthreads', 4);
end
```

