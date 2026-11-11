#!/bin/bash -e
# Copyright (c) 2024-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)

prev=$(pwd)
current=$(cd $(dirname $0); pwd)
cd $current/../deps

# ここにダウンロードしてきた外部ライブラリをビルドする処理を記述する。
# その際、シミュレータ本体やプラグインがリンクした外部ライブラリ(BoostやEigen等)を
# 使用できるようにするために、外部ライブラリのCMakeLists.txtをそのまま使用するのではなく、
# $current/../CMakeLists.txtとして配置したラッパーファイルを用いることとする。

# 上記のラッパーファイルを用いると、例えば外部ライブラリの名前を"ABC"としたとき、以下のような引数でcmakeコマンドを呼び出せばよい。
# 外部ライブラリに渡すオプションは必要に応じて追記すること。
#cmake_args=(
#    -S $PROJECT_ROOT_DIR/thirdParty
#    -B $BUILD_DIR/thirdParty/ABC
#    -DTHIRDPARTY_SOURCE_DIR=$PROJECT_ROOT_DIR/thirdParty/deps/ABC
#    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
#    -DCMAKE_INSTALL_PREFIX=$BUILD_DIR/thirdParty/install/
#)
#if [ -n "$ASRC_CMAKE_PREFIX_PATH" ]; then
#    cmake_args+=(
#        -DCMAKE_PREFIX_PATH="$ASRC_CMAKE_PREFIX_PATH"
#    )
#fi
#if [ -n "$MSYS_ROOT_DIR" ]; then
#    cmake_args+=(
#        -G "MSYS Makefiles"
#        -DMSYS_ROOT_DIR="$MSYS_ROOT_DIR"
#    )
#fi
#cmake "${cmake_args[@]}"

#cd $BUILD_DIR/thirdParty/ABC

# ビルドが必要ならばmakeを呼び出す。
#if [ "$(uname)" == 'Darwin' ]; then
#    make -j $(sysctl -n hw.physicalcpu)
#else
#    make -j $(nproc --all)
#fi

# ビルド不要なもの(ヘッダオンリーライブラリ等)は直接make installを呼び出す。
#make install
#cd $BUILD_DIR

cd $prev
