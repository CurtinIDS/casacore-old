
# Casacore

A suite of c++ libraries for radio astronomy data processing.


# Installation

## Obtaining the source
The original version of the code (and the relavant documentation) is maintained at:

```
$ git clone https://github.com/casacore/casacore
```


### Enabling Parallel IO

The code contains modules that enable multi-threaded access to mesurement sets.

To enable parallel IO, compile using: 

In the casacore source folder run:
```
mkdir build
cd build
cmake ..
make 
make install
```

there are various flags available to cmake to enable and disable options:
```
$ cmake -DUSE_FFTW3=ON -DDATA_DIR=/usr/share/casacore/data -DUSE_OPENMP=ON \
    -DUSE_HDF5=ON -DBUILD_PYTHON=ON -DUSE_THREADS=ON
```

