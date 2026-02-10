# Variance Deltas

This is a software package for constructing variance deltas, tree-structured visualizations of Bayesian posterior uncertainty. This project consists of three components:
1. **Factorization specification language:** An interpreter of a simple Stan-like language which allows users to:
  - Specify the factorization of their probability model.
  - Specify initial conditions for generating useful variance deltas.
2. **Variance deltas server:** The core application for generating and mutating variance deltas.
3. **Web-based interactive interface:** The main interface with which users can visualize, modify, and export generated variance deltas.

## Installation
Currently, this project can be built from source and installed on MacOS by running the `./build.sh && ./install.sh` from the root directory. In the near future, binary distributions will be provided for all major platforms. Building on other platforms is likely to work but is currently unsupported.

## Licensing
The `variance-deltas` package is provided under the GNU GPLv3 license. See `/LICENSE` for full terms.

This project depends on the [ranger](https://github.com/imbs-hl/ranger) C++ program for non-parametric random forest regression, which is licensed under the MIT license. See `/ranger/cpp_version/COPYING` for full license terms.