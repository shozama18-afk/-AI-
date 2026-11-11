#!/bin/bash
# Copyright (c) 2024-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)

prev=$(pwd)
current=$(cd $(dirname $0); pwd)
cd $current/.. #doc

export SelfRoot=$(cd $current/../..; pwd)
export SelfName=$(basename $(cd "${SelfRoot}"; pwd))

export Python3_INCLUDE_DIRS=$(python scripts/get_python_include_path.py)
export Python3_Numpy_INCLUDE_DIRS=$(python scripts/get_numpy_include_path.py)

echo "Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}"
echo "Python3_Numpy_INCLUDE_DIRS=${Python3_Numpy_INCLUDE_DIRS}"

doxygen

cd $prev
