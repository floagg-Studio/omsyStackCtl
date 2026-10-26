#!/bin/bash
set -e

gcc src/omsyd.c src/common.c -o build/omsyd
gcc src/omsyctl.c src/common.c -o build/omsyctl

# mv build/omsyctl build/omsyd to ./sys/bin/
# mv webinterface/ to ./usr/
# mv handler/ to ./sys/omsy/
# mv lib/* to ./sys/lib/omsy/