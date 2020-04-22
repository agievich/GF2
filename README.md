GF2: A C++ library for algebra over GF(2)
=========================================

[![Build Status](https://travis-ci.org/agievich/GF2.svg?branch=master)](https://travis-ci.org/agievich/GF2)
[![Coverity Static Analysis](https://scan.coverity.com/projects/9408/badge.svg)](https://scan.coverity.com/projects/agievich-gf2)
[![Coverage Analysis](https://codecov.io/gh/agievich/GF2/coverage.svg?branch=master)](https://codecov.io/gh/agievich/GF2?branch=master)

What is GF2?
-------------

GF2 is a C++ library which maintains the following algebraic structures 
over GF(2): 

- `WW` — binary words of arbitrary finite length;
- `ZZ` — binary words as unsigned integers;
- `MP` — multivariate polynomials over GF(2);
- `MI` — ideals in multivariate polynomials rings over GF(2);
- `Func` — various functions on binary words (including Boolean and vectorial Boolean).

GF2 extensively uses C++ templates.

Build
-----

    mkdir build
    cd build
    cmake [-DCMAKE_BUILD_TYPE={Release|Debug|Coverage|ASan|ASanDbg|MemSan|MemSanDbg|Check}] ..
    make
    [make test]
    [make install]

Build types (`Release` by default):
   
-  `Debug` — for debugging;
-  `Coverage` — test coverage;
-  `ASan`, `ASanDbg` — [address sanitizer](http://en.wikipedia.org/wiki/AddressSanitizer);
-  `MemSan`, `MemSanDbg` — [memory sanitizer](http://code.google.com/p/memory-sanitizer/);
-  `Check` — strict compile rules.

License
-------

GF2 is released under the terms of the MIT License. 
See [LICENSE](LICENSE) for more information.
