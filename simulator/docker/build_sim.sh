#!/bin/bash -e
prev=$pwd
current=$(cd $(dirname $0); pwd)
cd $current

cd ..
source /opt/data/venv/bin/activate
pip install -U "setuptools>=62.4"
python install_all.py -c "$@"
python install_all.py -p "$@"
python install_all.py -u "$@"

cd $prev
