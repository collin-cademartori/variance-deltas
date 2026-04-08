# Variance Deltas

This is a software package for constructing variance deltas, tree-structured visualizations of Bayesian posterior uncertainty. This project consists of three components:
1. **Factorization specification language:** An interpreter of a simple Stan-like language which allows users to:
  - Specify the factorization of their probability model.
  - Specify initial conditions for generating useful variance deltas.
2. **Variance deltas server:** The core application for generating and mutating variance deltas.
3. **Web-based interactive interface:** The main interface with which users can visualize, modify, and export generated variance deltas.

## Installation
Binaries for Windows, MacOS (Apple Silicon), and Linux can be found on the [releases](https://github.com/collin-cademartori/variance-deltas/releases) page. The `install.sh` and `install.ps1` scripts install and add the `vd` utility to the path on MacOS/Linux and Windows respectively.

This project can also be built from source and installed on MacOS by running the `./build.sh && ./install.sh` from the root directory. Building from source may also work on other platforms, but this is untested.

## Licensing
The `variance-deltas` package is provided under the GNU GPLv3 license. See `/LICENSE` for full terms.

This project depends on the [ranger](https://github.com/imbs-hl/ranger) C++ program for non-parametric random forest regression, which is licensed under the MIT license. See `/ranger/cpp_version/COPYING` for full license terms.