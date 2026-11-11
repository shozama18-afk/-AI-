#!/bin/bash -e
# workコンテナに入った直後に実行するスクリプト
# 仮想環境をworkspaceに作成してビルド済シミュレータをインストールする。
prev=$pwd
current=$(cd $(dirname $0); pwd)
cd $current

cd /workspace

# 仮想環境の作成
# 既に存在する場合は作成しない
if [[ ! -d ./venv ]]; then
    python3 -m venv venv
    sed -i '$asource /workspace/venv/bin/activate' ~/.bashrc
    source ./venv/bin/activate

    # シミュレータのインストール
    pip install ./dist/*.whl tensorboard torch==2.9.1 "ray[default,tune,rllib]<=2.46.0" opencv-python-headless "setuptools>=62.4"
else
    sed -i '$asource /workspace/venv/bin/activate' ~/.bashrc
    source ./venv/bin/activate
fi

cd $prev
