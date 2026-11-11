#!/bin/bash
# Copyright (c) 2024-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#
# generate_doc()にこのリポジトリ用のドキュメント生成コマンドを記述すること。
#
# 
# - RepoRoot  ドキュメント作成の起点となるリポジトリのパス
# - CoreRoot  シミュレータ本体のリポジトリのパス
# - SelfRoot  このリポジトリのパス
# - RootName  起点リポジトリの名前。\ref \mainpage_rootでそのトップページにリンクできる。
# - SelfName  このリポジトリの名前。必ずトップページに\anchor \mainpage_selfとしてアンカーを置くこと。\ref \mainpage_selfで自身のトップページを参照できる。

prev=$(pwd)
current=$(cd $(dirname $0); pwd)
cd $current/.. #doc

only_core=0
while (( $# > 0 ))
do 
    case $1 in
        # ...
        -c | --core)
            only_core=1
            ;;
        # ...
    esac
    shift
done

if [ -n "$RepoRoot" ]; then
    # RepoRootが定義済なら、自身をトップ階層としたドキュメント生成ではない
    echo "RepoRoot detected! '$RepoRoot'"
    is_top=0
else
    # RepoRootが未定義なら、自身をトップ階層としたドキュメント生成とみなす
    export RepoRoot=$(cd $current/../..; pwd)
    export RootName=$(basename $(cd "${RepoRoot}"; pwd))
    echo "RepoRoot not detected! Assume this is root repo. RepoRoot="
    is_top=1
    export CoreRoot=$(cd $current/../..; pwd)
fi

export SelfRoot=$(cd $current/../..; pwd)
export SelfName=$(basename $(cd "${SelfRoot}"; pwd))

export Python3_INCLUDE_DIRS=$(python scripts/get_python_include_path.py)
export Python3_Numpy_INCLUDE_DIRS=$(python scripts/get_numpy_include_path.py)

echo "Python3_INCLUDE_DIRS=${Python3_INCLUDE_DIRS}"
echo "Python3_Numpy_INCLUDE_DIRS=${Python3_Numpy_INCLUDE_DIRS}"

function generate_doc() {
    if [ $only_core = 1 ]; then
        # 本体部分のみ
        doxygen
    else
        ## 本体部分
        doxygen
        ## core plugin
        bash ../core_plugins/HandyRLUtility/doc/scripts/generate.sh
        bash ../core_plugins/MatchMaker/doc/scripts/generate.sh
        bash ../core_plugins/rayUtility/doc/scripts/generate.sh
        bash ../core_plugins/Supervised/doc/scripts/generate.sh
        bash ../core_plugins/torch_truncnorm/doc/scripts/generate.sh
        ## user plugin
        bash ../user_plugins/BasicAirToAirCombatModels01/doc/scripts/generate.sh
        bash ../user_plugins/BasicAgentUtility/doc/scripts/generate.sh
        bash ../user_plugins/r8_contest_models/doc/scripts/generate.sh
        ## sample
        bash ../sample/modules/cpp_template/doc/scripts/generate.sh
        bash ../sample/modules/py_template/doc/scripts/generate.sh
        bash ../sample/modules/r8_contest_sample/doc/scripts/generate.sh
    fi
}

if [ $is_top = 1 ]; then
    # 自身がトップなら最終生成物フォルダを作る
    mkdir -p html
    mkdir -p tags

    # 1巡目はタグファイルの生成
    generate_doc $is_top

    # 1巡目の後、生成されたタグファイルを列挙して2巡目のDoxyfileに供給
    TagFiles=""
    for f in tags/*.tag; do
        name=$(basename $f .tag)
        
        TagFiles="${TagFiles}\$(RepoRoot)/doc/tags/${name}.tag=../${name} "
    done
    export TagFiles="${TagFiles}"
    
    # 2巡目はタグファイルを利用して相互参照の解決
    generate_doc $is_top
else
    # 自身がトップでないならトップのリポジトリが2巡呼んでくれるので1回だけ実行する
    generate_doc $is_top
fi

cd $prev
