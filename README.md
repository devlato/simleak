# simleak

Simulates a memory leak by consuming memory in small chunks. Suitable for testing performance and behavior of software in cases of having almost no available RAM.

Tested on macOS only so far, but build for Linux is coming.


# Installation

As of now, there's no way to avoid building (which is quite simple though). Just clone the repo, run `make build`
and you'll get the binary in `target/build/simleak`. Before running the command, make sure you have `g++` and `make` installed.


# Use

```
$ simleak -h
Simulates a memory leak by consuming memory in small chunks
Usage:
  simleak [OPTION...]

  -m, --memory-limit arg  Sets memory limit in MBs (default: 32768)
  -c, --chunk-size arg    Sets chunk size in MBs (default: 32)
  -v, --verbose           Enable verbose execution log
  -z, --fill-with-zeroes  Fill the allocated memory with zeroes instead of
                          random values (faster)
  -h, --help              Print this help message
```


# Dependencies

This small binary is using [cxxopts](https://github.com/jarro2783/cxxopts) for parsing command-line arguments.
Thanks to [@jarro2783](https://github.com/jarro2783) for creating and maintaining it.


# License

This software is distributed under the MIT license. See LICENSE.md for the details.
