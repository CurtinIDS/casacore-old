
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

To enable parallel IO, compile using (relavant sections of cmake args): 
```
$ cmake -DCMAKE_EXE_LINKER_FLAGS="-lrt" -DCMAKE_CXX_FLAGS="-qopenmp -DPARIO" -DCMAKE_EXE_LINKER_FLAGS="-lpthread -lrt -lboost_regex -lboost_thread -qopenmp -lboost_filesystem -lboost_system -DPARIO"-DUSE_STACKTRACE=ON -DUSE_THREADS=ON *<your_cmake_switches>* 
```


