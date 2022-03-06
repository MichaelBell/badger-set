# The Badger Set - small C++ programs for the Badger2040

The Low Power Badger runs the RP2040 at a slow clock and turns off unused clock domains to minimize power usage.
It also provides a wrapper around flash storage to allow state to be preserved over system halt.

Current programs are:
* __badger-image__ A modified version of the original Pimoroni badger image example
* __badger-count__ A counter using the persistent flash storage
* __badger-connect__ A game of Connect 4

## Setup

This repo has the pimoroni-pico repo as a submodule.  To set that up do

    git submodule update --init --recursive

## Build

As normal do

    mkdir build
    cd build
    cmake ..

It is best not to run make in the top level directory as that will build the whole of the pimoroni-pico repo.
Instead change to the directory if the individual program you want to run and build it there, eg:

    cd badger-image
    make -j4

