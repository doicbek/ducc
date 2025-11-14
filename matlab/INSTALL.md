# DUCC0 MATLAB Wrapper Installation Guide

## Prerequisites

1. **MATLAB** (R2018b or later)
   - Must have Python interface support
   - Check with: `pyversion`

2. **Python** (3.8 or later)
   - Verify installation: `python --version`
   - Ensure Python is accessible from MATLAB

3. **ducc0 Python Package**
   - Install via pip: `pip install ducc0`
   - Or from source: `pip install --no-binary ducc0 ducc0`

## Installation Steps

### Step 1: Install ducc0 Python Package

**Option A: Install Pre-compiled Binary (Recommended for Quick Setup)**
```bash
pip install ducc0
```

**Option B: Install from Source (Recommended for Best Performance)**
```bash
pip install --no-binary ducc0 --user ducc0
```

Note: Compilation from source requires a C++17 compiler and can take several minutes.

### Step 2: Configure MATLAB Python Interface

1. Start MATLAB

2. Check current Python configuration:
   ```matlab
   pyversion
   ```

3. If Python is not configured or wrong version, set it:
   ```matlab
   % On Linux/Mac:
   pyversion('/usr/bin/python3')
   
   % On Windows:
   pyversion('C:\Python39\python.exe')
   
   % Or let MATLAB find it automatically:
   pyversion  % Shows available Python versions
   ```

4. Verify ducc0 can be imported:
   ```matlab
   try
       py.importlib.import_module('ducc0');
       fprintf('ducc0 module found successfully!\n');
   catch ME
       fprintf('Error: %s\n', ME.message);
       fprintf('Please install ducc0: pip install ducc0\n');
   end
   ```

### Step 3: Add MATLAB Wrapper to Path

**Option A: Add to MATLAB Path Permanently**

1. In MATLAB, go to: Home → Set Path
2. Click "Add Folder..."
3. Navigate to and select the `matlab` directory in the DUCC repository
4. Click "Save"

**Option B: Add to Path in Script/Session**

Add this line to your MATLAB startup script or run it at the beginning of each session:
```matlab
addpath('/path/to/ducc/matlab')
```

**Option C: Use as Package (Recommended)**

If you want to use the package structure, add the parent directory:
```matlab
addpath('/path/to/ducc')
```

Then use: `import ducc0.*`

### Step 4: Verify Installation

Run this test script:
```matlab
% Test basic functionality
try
    import ducc0.*
    
    % Test FFT
    x = randn(64, 64) + 1i*randn(64, 64);
    y = ducc0.fft.c2c(x);
    fprintf('FFT test passed!\n');
    
    % Test SHT
    lmax = 16;
    nalm = (lmax+1)*(lmax+2)/2;
    alm = randn(1, nalm) + 1i*randn(1, nalm);
    map = ducc0.sht.synthesis_2d(alm, lmax, 'ntheta', 17, 'nphi', 34);
    fprintf('SHT test passed!\n');
    
    fprintf('\nInstallation successful!\n');
catch ME
    fprintf('Error during test: %s\n', ME.message);
    fprintf('Stack trace:\n');
    for i = 1:length(ME.stack)
        fprintf('  %s at line %d\n', ME.stack(i).file, ME.stack(i).line);
    end
end
```

## Troubleshooting

### Issue: "Python module 'ducc0' not found"

**Solution:**
1. Verify Python installation:
   ```matlab
   pyversion
   ```

2. Install ducc0 in the Python environment MATLAB is using:
   ```bash
   # Use the Python executable that MATLAB sees
   /path/to/python -m pip install ducc0
   ```

3. Restart MATLAB after installation

### Issue: "Array conversion errors"

**Solution:**
- Ensure arrays are numeric (not cell arrays or structures)
- Convert to appropriate type:
  ```matlab
  x = double(x);  % For real arrays
  x = complex(x);  % For complex arrays
  ```

### Issue: "Wrong Python version"

**Solution:**
1. Check available Python versions:
   ```matlab
   pyversion
   ```

2. Set correct version:
   ```matlab
   pyversion('/path/to/correct/python')
   ```

3. Restart MATLAB

### Issue: "Performance is slow"

**Solution:**
1. Install ducc0 from source with optimizations:
   ```bash
   pip install --no-binary ducc0 --user ducc0
   ```

2. Use multi-threading:
   ```matlab
   result = ducc0.fft.c2c(x, 'nthreads', 4);
   ```

3. Pre-allocate output arrays when possible

## Platform-Specific Notes

### Windows
- Ensure Python is added to system PATH
- May need to run MATLAB as administrator for path changes
- Use forward slashes or double backslashes in paths: `'C:/Python39/python.exe'`

### Linux/Mac
- Python 3 is typically available as `python3`
- May need to install Python development headers:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install python3-dev
  
  # Mac (with Homebrew)
  brew install python3
  ```

## Next Steps

After successful installation:
1. Read the main README.md for usage examples
2. Explore the module documentation: `help ducc0.fft.c2c`
3. Check out the DUCC Python documentation: https://mtr.pages.mpcdf.de/ducc

## Getting Help

- Check the main README.md for usage examples
- Review DUCC Python documentation
- Check MATLAB's Python interface documentation: `doc pyversion`
- For DUCC-specific issues, refer to the main DUCC repository

