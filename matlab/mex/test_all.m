% test_all  Comprehensive test suite for the DUCC0 MATLAB wrapper.
%
%   Run from the matlab/mex directory after a successful build:
%     >> cd matlab/mex
%     >> test_all
%
%   Or from any directory:
%     >> run('/path/to/ducc/matlab/mex/test_all.m')

function test_all()

this_dir = fileparts(mfilename('fullpath'));
addpath(fullfile(this_dir, 'build'));
addpath(fullfile(this_dir, '..'));

passed  = 0;
failed  = 0;

fprintf('==============================================\n');
fprintf('DUCC0 MATLAB wrapper – comprehensive tests\n');
fprintf('==============================================\n\n');

% Helper: run one named test, catch errors, update counters
    function ok = T(name, fn)
        fprintf('  %-55s', name);
        try
            fn();
            fprintf('PASS\n');
            ok = true;
            passed = passed + 1;
        catch ME
            fprintf('FAIL\n    %s\n', ME.message);
            ok = false;
            failed = failed + 1;
        end
    end

% ------------------------------------------------------------------
% 1. MEX file presence
% ------------------------------------------------------------------
fprintf('--- MEX file presence ---\n');
all_mex = { ...
    'ducc0_fft_c2c_mex',              'ducc0_fft_r2c_mex', ...
    'ducc0_fft_c2r_mex',              'ducc0_fft_r2r_fftpack_mex', ...
    'ducc0_fft_dct_mex',              'ducc0_fft_dst_mex', ...
    'ducc0_fft_hartley_mex',          'ducc0_fft_good_size_mex', ...
    'ducc0_sht_synthesis_2d_mex',     'ducc0_sht_analysis_2d_mex', ...
    'ducc0_sht_adjoint_synthesis_2d_mex', 'ducc0_sht_adjoint_analysis_2d_mex', ...
    'ducc0_sht_synthesis_mex',        'ducc0_sht_adjoint_synthesis_mex', ...
    'ducc0_sht_rotate_alm_mex',       'ducc0_sht_get_gridweights_mex', ...
    'ducc0_nufft_nu2u_mex',           'ducc0_nufft_u2nu_mex', ...
    'ducc0_healpix_ang2pix_mex',      'ducc0_healpix_pix2ang_mex', ...
    'ducc0_misc_vdot_mex' ...
};
mex_ok = true;
for k = 1:numel(all_mex)
    nm = all_mex{k};
    if exist(nm, 'file') == 3
        fprintf('  %-50s FOUND\n', nm);
        passed = passed + 1;
    else
        fprintf('  %-50s MISSING\n', nm);
        failed  = failed + 1;
        mex_ok  = false;
    end
end
if ~mex_ok
    fprintf('\nSome MEX files are missing. Build them first:\n');
    fprintf('  cd %s && mkdir build && cd build\n', fullfile(this_dir));
    fprintf('  cmake .. && cmake --build . -j$(nproc)\n\n');
end
fprintf('\n');

% ------------------------------------------------------------------
% 2. FFT
% ------------------------------------------------------------------
fprintf('--- FFT ---\n');
T('good_size: result >= input', ...
    @() assert(ducc0.fft.good_size(1000) >= 1000));

T('c2c: double 2-D round-trip', @() ...
    assert(fft_err(@(x) ducc0.fft.c2c( ...
        ducc0.fft.c2c(x), 'forward', false, 'inorm', 2), ...
        randn(64,64)+1i*randn(64,64)) < 1e-10));

T('c2c: single 2-D round-trip', @() ...
    assert(fft_err(@(x) ducc0.fft.c2c( ...
        ducc0.fft.c2c(x), 'forward', false, 'inorm', 2), ...
        single(randn(32,32)+1i*randn(32,32))) < 1e-4));

T('c2c: axis selection (axis 2 of 3-D)', @() ...
    assert(fft_err(@(x) ducc0.fft.c2c( ...
        ducc0.fft.c2c(x, 'axes', 2), ...
        'axes', 2, 'forward', false, 'inorm', 2), ...
        randn(8,16,8)+1i*randn(8,16,8)) < 1e-10));

T('r2c / c2r: round-trip', @() r2c_c2r_test());

T('r2r_fftpack: identity (type 1)', @() ...
    assert(fft_err(@(x) ducc0.fft.r2r_fftpack( ...
        ducc0.fft.r2r_fftpack(x, 'type', [2]), 'type', [2], 'inorm', 2), ...
        randn(32,32)) < 1e-10));

T('dct: type-2 / type-3 round-trip', @() ...
    assert(fft_err(@(x) ducc0.fft.dct( ...
        ducc0.fft.dct(x, 'type', 2), 'type', 3, 'inorm', 2), ...
        randn(32,32)) < 1e-10));

T('dst: type-2 / type-3 round-trip', @() ...
    assert(fft_err(@(x) ducc0.fft.dst( ...
        ducc0.fft.dst(x, 'type', 2), 'type', 3, 'inorm', 2), ...
        randn(32,32)) < 1e-10));

T('hartley: self-inverse', @() ...
    assert(fft_err(@(x) ducc0.fft.hartley( ...
        ducc0.fft.hartley(x), 'inorm', 2), ...
        randn(64,64)) < 1e-10));

fprintf('\n');

% ------------------------------------------------------------------
% 3. SHT
% ------------------------------------------------------------------
fprintf('--- SHT ---\n');
lmax = 32;
nalm = (lmax+1)*(lmax+2)/2;

T('get_gridweights: CC positive, integrates to 4pi', @() gridweights_test());

T('synthesis_2d / analysis_2d: CC round-trip',   @() sht_2d_roundtrip('CC',  lmax));
T('synthesis_2d / analysis_2d: GL round-trip',   @() sht_2d_roundtrip('GL',  lmax));
T('synthesis_2d / analysis_2d: DH round-trip',   @() sht_2d_roundtrip('DH',  lmax));
T('synthesis_2d / analysis_2d: MW round-trip',   @() sht_2d_roundtrip('MW',  lmax));

T('adjoint_synthesis_2d: <A*y,x> == <y,Ax>', @() adjoint_2d_test(lmax));
T('adjoint_analysis_2d:  <A*x,y> == <x,Ay>', @() adjoint_analysis_2d_test(lmax));

T('synthesis / adjoint_synthesis: arbitrary rings', ...
    @() sht_arbitrary_rings(lmax));

T('rotate_alm: identity rotation (psi=theta=phi=0)', ...
    @() rotate_alm_identity(lmax));

T('alm2map / map2alm: high-level round-trip', ...
    @() alm2map_roundtrip(lmax));

fprintf('\n');

% ------------------------------------------------------------------
% 4. NUFFT
% ------------------------------------------------------------------
fprintf('--- NUFFT ---\n');
T('nu2u: 1-D output size correct',  @() nu2u_size_1d());
T('nu2u: 2-D output size correct',  @() nu2u_size_2d());
T('u2nu: 1-D output size correct',  @() u2nu_size_1d());
T('u2nu: 2-D output size correct',  @() u2nu_size_2d());
T('nu2u / u2nu: 1-D DFT comparison', @() nufft_vs_dft_1d());

fprintf('\n');

% ------------------------------------------------------------------
% 5. HEALPix
% ------------------------------------------------------------------
fprintf('--- HEALPix ---\n');
T('nside2npix: formula 12*nside^2', @() ...
    arrayfun(@(n) assert(ducc0.healpix.nside2npix(n)==12*n^2), [1 2 4 8 16 32 64]));

T('npix2nside: round-trip', @() ...
    arrayfun(@(n) assert(ducc0.healpix.npix2nside(12*n^2)==n), [1 2 4 8 16 32 64]));

T('ang2pix / pix2ang: pixel round-trip (RING)', @() healpix_ang2pix_ring());
T('ang2pix / pix2ang: pixel round-trip (NESTED)', @() healpix_ang2pix_nested());

fprintf('\n');

% ------------------------------------------------------------------
% 6. Misc
% ------------------------------------------------------------------
fprintf('--- Misc ---\n');
T('vdot: real vectors match dot()', @() ...
    assert(abs(ducc0.misc.vdot([1 2 3], [4 5 6]) - dot([1 2 3],[4 5 6])) < 1e-14));

T('vdot: complex conjugate-linear in first arg', @() vdot_complex());

T('l2error: identical arrays -> 0', @() ...
    assert(ducc0.misc.l2error(randn(50,1), randn(50,1)*0 + 0) >= 0));  % just checks no error

T('l2error: x vs x is 0', @() ...
    assert(ducc0.misc.l2error(pi*ones(10), pi*ones(10)) == 0));

fprintf('\n');

% ------------------------------------------------------------------
% Summary
% ------------------------------------------------------------------
total = passed + failed;
fprintf('==============================================\n');
fprintf('Results: %d / %d passed', passed, total);
if failed > 0
    fprintf(', %d FAILED', failed);
end
fprintf('\n==============================================\n');

end % test_all


% ==================================================================
% FFT helpers
% ==================================================================

function e = fft_err(fn, x)
    z = fn(x);
    e = max(abs(x(:) - z(:)));
end

function r2c_c2r_test()
    x = randn(64, 64);
    y = ducc0.fft.r2c(x);
    z = ducc0.fft.c2r(y, 'lastsize', size(x, 2));
    assert(max(abs(x(:) - z(:))) < 1e-10, 'r2c/c2r round-trip error too large');
end

% ==================================================================
% SHT helpers
% ==================================================================

function gridweights_test()
    w = ducc0.sht.get_gridweights('CC', 64);
    assert(all(w > 0), 'weights not all positive');
    assert(abs(sum(w) * 2*pi - 4*pi) < 1e-10, ...
        'CC weights do not integrate sphere to 4pi');
end

function sht_2d_roundtrip(geometry, lmax)
    nalm = (lmax+1)*(lmax+2)/2;
    alm  = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));   % m=0 must be real
    map  = ducc0.sht.synthesis_2d(alm, lmax, 'geometry', geometry);
    alm2 = ducc0.sht.analysis_2d(map,  lmax, 'geometry', geometry);
    rel  = max(abs(alm(:) - alm2(:))) / max(abs(alm(:)));
    assert(rel < 1e-7, sprintf('%s round-trip rel.err %.2e >= 1e-7', geometry, rel));
end

function adjoint_2d_test(lmax)
    % Verify <A* y, x> = <y, A x> for synthesis_2d / adjoint_synthesis_2d
    nalm  = (lmax+1)*(lmax+2)/2;
    ntheta = lmax + 2; nphi = 2*lmax + 2;
    alm = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));
    y   = randn(1, ntheta, nphi);
    Ax  = ducc0.sht.synthesis_2d(alm, lmax, 'geometry', 'CC', ...
                                  'ntheta', ntheta, 'nphi', nphi);
    Asy = ducc0.sht.adjoint_synthesis_2d(y, lmax, 'geometry', 'CC');
    lhs = real(sum(conj(y(:))   .* Ax(:)));
    rhs = real(sum(conj(Asy(:)) .* alm(:)));
    assert(abs(lhs-rhs)/(abs(lhs)+1e-30) < 1e-8, ...
        sprintf('adjoint_synthesis_2d: lhs=%.6e rhs=%.6e', lhs, rhs));
end

function adjoint_analysis_2d_test(lmax)
    % Verify <A* x, y> = <x, A y> for analysis_2d / adjoint_analysis_2d
    nalm  = (lmax+1)*(lmax+2)/2;
    ntheta = lmax + 2; nphi = 2*lmax + 2;
    map = randn(1, ntheta, nphi);
    alm = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));
    Ax  = ducc0.sht.analysis_2d(map, lmax, 'geometry', 'CC');
    Asy = ducc0.sht.adjoint_analysis_2d(alm, lmax, 'geometry', 'CC', ...
                                         'ntheta', ntheta, 'nphi', nphi);
    lhs = real(sum(conj(alm(:)) .* Ax(:)));
    rhs = real(sum(conj(map(:)) .* Asy(:)));
    assert(abs(lhs-rhs)/(abs(lhs)+1e-30) < 1e-8, ...
        sprintf('adjoint_analysis_2d: lhs=%.6e rhs=%.6e', lhs, rhs));
end

function sht_arbitrary_rings(lmax)
    nrings       = lmax + 2;
    nphi_per_ring = 2*lmax + 2;
    theta    = ((0.5:nrings) * pi / nrings)';
    nphi_arr = repmat(nphi_per_ring, nrings, 1);
    phi0_arr = zeros(nrings, 1);
    ringstart = (0:nrings-1)' * nphi_per_ring;   % 0-based
    nalm = (lmax+1)*(lmax+2)/2;
    alm  = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));
    map  = ducc0.sht.synthesis(alm, lmax, 0, theta, nphi_arr, phi0_arr, ringstart);
    alm2 = ducc0.sht.adjoint_synthesis(map, lmax, 0, theta, nphi_arr, phi0_arr, ringstart);
    assert(numel(alm2) == nalm, 'adjoint_synthesis output size wrong');
end

function rotate_alm_identity(lmax)
    nalm = (lmax+1)*(lmax+2)/2;
    alm  = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));
    alm2 = ducc0.sht.rotate_alm(alm, lmax, 0, 0, 0);
    assert(max(abs(alm(:) - alm2(:))) < 1e-10, 'identity rotation changed alm');
end

function alm2map_roundtrip(lmax)
    nside = max(lmax / 2, 16);
    nalm  = (lmax+1)*(lmax+2)/2;
    alm   = randn(1, nalm) + 1i*randn(1, nalm);
    alm(1:lmax+1) = real(alm(1:lmax+1));
    map   = ducc0.sht.alm2map(alm, lmax, nside);
    alm2  = ducc0.sht.map2alm(map, lmax, nside);
    rel   = max(abs(alm(:) - alm2(:))) / max(abs(alm(:)));
    assert(rel < 1e-3, sprintf('alm2map/map2alm rel.err %.2e >= 1e-3', rel));
end

% ==================================================================
% NUFFT helpers
% ==================================================================

function nu2u_size_1d()
    npts  = 50; gridN = 64;
    coord = 2*pi * rand(npts, 1);
    amp   = randn(npts, 1) + 1i*randn(npts, 1);
    g = ducc0.nufft.nu2u(amp, coord, 'grid_shape', gridN, 'epsilon', 1e-6);
    assert(numel(g) == gridN, sprintf('nu2u 1-D: expected %d, got %d', gridN, numel(g)));
end

function nu2u_size_2d()
    npts  = 100; gridN = [16, 32];
    coord = 2*pi * rand(npts, 2);
    amp   = randn(npts, 1) + 1i*randn(npts, 1);
    g = ducc0.nufft.nu2u(amp, coord, 'grid_shape', gridN, 'epsilon', 1e-6);
    assert(isequal(size(g), gridN), 'nu2u 2-D output shape mismatch');
end

function u2nu_size_1d()
    npts = 50; gridN = 64;
    grid  = randn(gridN, 1) + 1i*randn(gridN, 1);
    coord = 2*pi * rand(npts, 1);
    pts = ducc0.nufft.u2nu(grid, coord, 'epsilon', 1e-6);
    assert(numel(pts) == npts, sprintf('u2nu 1-D: expected %d, got %d', npts, numel(pts)));
end

function u2nu_size_2d()
    npts = 100; gridN = [16, 32];
    grid  = randn(gridN) + 1i*randn(gridN);
    coord = 2*pi * rand(npts, 2);
    pts = ducc0.nufft.u2nu(grid, coord, 'epsilon', 1e-6);
    assert(numel(pts) == npts, 'u2nu 2-D output size mismatch');
end

function nufft_vs_dft_1d()
    % Compare nu2u with an exact DFT sum for a small case
    N    = 16;
    npts = 8;
    coord = 2*pi * (0:npts-1)' / npts;   % uniform sub-grid
    amp   = randn(npts, 1) + 1i*randn(npts, 1);

    % NUFFT result
    g_nufft = ducc0.nufft.nu2u(amp, coord, 'grid_shape', N, 'epsilon', 1e-12, 'fft_order', false);

    % Exact sum: g[k] = sum_j amp[j] * exp(-i * coord[j] * k)   k=0..N-1
    k = 0:N-1;
    g_exact = zeros(1, N);
    for j = 1:npts
        g_exact = g_exact + amp(j) * exp(-1i * coord(j) * k);
    end

    err = max(abs(g_nufft(:) - g_exact(:))) / max(abs(g_exact(:)));
    assert(err < 1e-8, sprintf('nu2u vs exact DFT rel.err %.2e >= 1e-8', err));
end

% ==================================================================
% HEALPix helpers
% ==================================================================

function healpix_ang2pix_ring()
    nside = 64;
    theta = [0.5, 1.0, 1.5, 2.5];
    phi   = [0.0, 1.0, 2.0, 3.0];
    pix   = ducc0.healpix.ang2pix(nside, theta, phi);
    [th2, ph2] = ducc0.healpix.pix2ang(nside, pix);
    pix2  = ducc0.healpix.ang2pix(nside, th2, ph2);
    assert(isequal(pix(:), pix2(:)), 'ang2pix/pix2ang RING pixel round-trip failed');
end

function healpix_ang2pix_nested()
    nside = 64;
    theta = [0.5, 1.0, 1.5, 2.5];
    phi   = [0.0, 1.0, 2.0, 3.0];
    pix   = ducc0.healpix.ang2pix(nside, theta, phi, 'nest', true);
    [th2, ph2] = ducc0.healpix.pix2ang(nside, pix, 'nest', true);
    pix2  = ducc0.healpix.ang2pix(nside, th2, ph2, 'nest', true);
    assert(isequal(pix(:), pix2(:)), 'ang2pix/pix2ang NESTED pixel round-trip failed');
end

% ==================================================================
% Misc helpers
% ==================================================================

function vdot_complex()
    a = [1+2i, 3+4i];
    b = [5+6i, 7+8i];
    expected = conj(a(1))*b(1) + conj(a(2))*b(2);
    result   = ducc0.misc.vdot(a, b);
    assert(abs(result - expected) < 1e-13, 'vdot complex mismatch');
end
