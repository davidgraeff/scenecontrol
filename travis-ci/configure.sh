#!/bin/sh
set -e

mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ../ && exit 0