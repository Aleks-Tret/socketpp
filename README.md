# socketpp

[![Build Status](https://travis-ci.org/bcachet/socketpp.png)](https://travis-ci.org/bcachet/socketpp)

## Build environment

 We use [cmake](http://cmake.org) to build library and associated tests.

 We use [Catch](https://github.com/philsquared/Catch) and [HippoMock](http://www.assembla.com/spaces/hippomocks/) libraries to write our tests and our specs.

 We use [Sphinx](http://sphinx.pocoo.org) to generate our documentation.

## Prepare your build environment

You will need to install cmake and Sphinx tools.

All other tools will be installed inside *thirdparty* directory as soon as you launch the **make prepare** command.

#### Debian/Ubuntu

```bash
apt-get install cmake
./tests/run-tests.sh
```
#### Mac OS X

You can easily install needed libraries with [Homebrew](http://mxcl.github.com/homebrew/):

```bash
brew install cmake
./tests/run-tests.sh
```
