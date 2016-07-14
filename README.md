GF2: A C++ library for algebra over GF(2)
=========================================

[![Build Status](https://travis-ci.org/agievich/GF2.svg?branch=master)](https://travis-ci.org/agievich/GF2)

What is GF2?
-------------

GF2 is a C++ library which maintains the following algebraic structures 
over GF(2): 

1. Word: words in GF(2) of arbitrary finite length.
2. ZZ: words in GF(2) as unsigned integers.
3. Monom: monomials in GF(2)[x0,x1,...].
4. MPoly: polynomials in GF(2)[x0,x1,...].
5. Ideal: ideals in GF(2)[x0,x1,...].
6. Func: functions on words (including Boolean and vectorial Boolean).

GF2 extensively uses C++ templates.

Build
-----

    mkdir build
    cd build
    cmake [-DCMAKE_BUILD_TYPE={Release|Debug|Coverage|ASan|ASanDbg|MemSan|MemSanDbg|Check}] ..
    make
    [make test]
    [make install]

Build types (Release by default):
   
*  Coverage -- test coverage,
*  ASan, ASanDbg -- [address sanitizer](http://en.wikipedia.org/wiki/AddressSanitizer),
*  MemSan, MemSanDbg -- [memory sanitizer](http://code.google.com/p/memory-sanitizer/),
*  Check -- strict compile rules.

License
-------

GF2 is released under the terms of the MIT License. 
See [LICENSE](LICENSE) for more information.
