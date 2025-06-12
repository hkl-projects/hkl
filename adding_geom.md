adding diffractometer geometries with basic testing

using an E4CV geometry with addition gamma axis as example, called "E4CVG"
"E4CVG2" is the same but without constraint modes

add file defining geometry hkl/hkl-engine-e4cvg.c
add test file tests/hkl-e4cgv-test-t.c
add line hkl-e4cgv-test-t to tests/Makefile.am, at end of "all\_tests" 
add geometry name to hkl/api2/hkl.h, hkl/api2/hkl.c
repeat for E4CVG2

```bash
autoreconf -vif
```

if not using hkl IOC:
```bash
./configure --disable-binoculars
```

if using hkl IOC (and want new geometry in the IOC):
```bash
./configure --disable-binoculars --enable-introspection
```

```bash
make check
```

