#!/bin/bash
set -e errexit

rm -rf cscope
cd src
make clobber
