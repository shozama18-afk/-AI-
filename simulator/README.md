ASRCAISim1 第5回空戦AIチャレンジ配布版
ASRCAISim1 for the 5th Air Combat AI Challenge edition

本バージョン(第5回空戦AIチャレンジ配布版)は第5回空戦AIチャレンジの参加規約及び情報公開ポリシー
に規定される条件で利用許諾されています。
詳細はhttps://signate.jp/competitions/1635を参照してください。

The redistribution and use of this edition of this software is permitted and constrained
under the terms and policy of the 5th Air Combat AI Challenge.
Please see https://signate.jp/competitions/1635 for detail.

# 環境構築方法

- [環境構築方法](#環境構築方法)
  - [推奨動作環境(主要な項目のみ)](#推奨動作環境主要な項目のみ)
  - [インストール方法(Dockerを使用する場合)](#インストール方法dockerを使用する場合)
    - [ユーザ情報の指定](#ユーザ情報の指定)
    - [ベースイメージの指定](#ベースイメージの指定)
    - [シミュレータ及びプラグインのビルド](#シミュレータ及びプラグインのビルド)
    - [作業用コンテナの初期設定](#作業用コンテナの初期設定)
    - [一部のみ更新した場合の個別ビルド](#一部のみ更新した場合の個別ビルド)
  - [インストール方法(ホスト側で直接インストールする場合)](#インストール方法ホスト側で直接インストールする場合)
    - [Python仮想環境の起動](#python仮想環境の起動)
    - [setuptoolsのインストール](#setuptoolsのインストール)
    - [シミュレータのビルド・インストール](#シミュレータのビルドインストール)
    - [深層学習用フレームワークのインストール](#深層学習用フレームワークのインストール)
    - [第5回空戦AIチャレンジ用サンプルのビルド・インストール](#第5回空戦aiチャレンジ用サンプルのビルドインストール)
    - [ドキュメントの生成](#ドキュメントの生成)
      - [Dockerを使用する場合](#dockerを使用する場合)
      - [Dockerを使用しない場合](#dockerを使用しない場合)
        - [Doxygenのインストール](#doxygenのインストール)
        - [ドキュメント生成スクリプトの実行](#ドキュメント生成スクリプトの実行)
    - [install\_all.pyのオプション一覧](#install_allpyのオプション一覧)
  - [第5回空戦AIチャレンジ用サンプルの動作確認 {#page\_r8\_contest\_getting\_started}](#第5回空戦aiチャレンジ用サンプルの動作確認-page_r8_contest_getting_started)
    - [シミュレータ本体の動作確認 {#section\_installation\_check\_simulator}](#シミュレータ本体の動作確認-section_installation_check_simulator)
    - [HandyRLを用いた学習のサンプル {#section\_installation\_check\_HandyRL\_sample}](#handyrlを用いた学習のサンプル-section_installation_check_handyrl_sample)
      - [学習の実施 {#section\_installation\_check\_HandyRL\_train}](#学習の実施-section_installation_check_handyrl_train)
      - [学習済モデルの評価 {#section\_installation\_check\_HandyRL\_evaluation}](#学習済モデルの評価-section_installation_check_handyrl_evaluation)
  - [アンインストール方法](#アンインストール方法)
    - [uninstall\_all.pyのオプション一覧](#uninstall_allpyのオプション一覧)

## 推奨動作環境(主要な項目のみ)

本シミュレータの推奨動作環境は以下の通りである。

- OS : Ubuntu 22.04 LTS
- Boost : 1.88.0
- cereal : 1.3.2
- CMake : 3.22.1 (>=3.10)
- Doxygen : 1.14.0 (取扱説明書を生成する場合のみ)
- Eigen : 3.4.0
- gcc : 11.4.0 (C++20)
- gymnasium : 0.28.1〜1.0.0 (rayのバージョンに依存する)
- Magic Enum C++ : 0.9.7
- nlohmann's json : 3.12.0
- NLopt : 2.7.1
- pybind11 : 2.13.6
- Python : 3.10
- PyTorch : 2.7.0 (>=2.1.0)
- ray[default,tune,rllib] : 2.7.0〜2.46.0
- thread-pool : 5.0.0

## インストール方法(Dockerを使用する場合)

### ユーザ情報の指定

`docker-compose.yml`の各サービスのbuild argsについて`USER_ID`, `GROUP_ID`, `REMOTE_USER`, `REMOTE_GROUP`を任意の値に設定する。
少なくともIDはホスト側での自身のID(`id -u`及び`id -g`)と一致させることを推奨する。

### ベースイメージの指定

`docker-compose.yml`の各サービスのbuild argsについて`BASE_IMAGE`と`BASE_TAG`を任意の値に設定する。
GPUが不要なら例えば`ubuntu`, `22.04`とすればよい。

### シミュレータ及びプラグインのビルド

`docker-compose.yml`の`build`サービスを用いるとシミュレータ及びプラグインをビルドし、`./dist`に格納することができる。ビルドには長時間かかるため、もしビルド済のwheelを持っている場合は手動で`./dist`に配置してこの手順を省略することを推奨する。ただし、ドキュメントの生成を行う場合はこの手順を実行して`build`のイメージを生成しておく必要がある。

```bash
# ビルド
docker compose build build

# ビルド済wheelをホスト側の./distにコピー
docker compose run --rm -u $(id -u):$(id -g) build
```

### 作業用コンテナの初期設定

`docker-compose.yml`の`work`サービスを学習・評価用の作業用コンテナとして用いることを想定している。
リモートデスクトップの追加やVS Codeとの連携等、必要に応じて適宜カスタマイズして使用すること。

```bash
# サービスの起動
docker compose up -d work

# コンテナに入る
docker exec -it work bash
```

上記のコマンドにより、このディレクトリが`/workspace`にマウントされ、`/workspace`がカレントディレクトリになった状態でコンテナに入ることができる。初回起動後は`workspace_setup.sh`を実行することで`/workspace/venv`に仮想環境を作成してその中にシミュレータ及びプラグインがインストールされる。

```bash
# 仮想環境の作成とインストール
bash ./workspace_setup.sh
```

### 一部のみ更新した場合の個別ビルド

上記のDockerによるビルドでは、`docker/build_sim.sh`によりこのリポジトリに含まれる全てのプラグインがビルドされるため時間がかかるが、一部のみ更新した場合等は、これらのスクリプト中で`install_all.py`の呼び方を変えると個別にビルド、インストールできる。`install_all.py`で指定可能なオプションは[こちら](#install_allpyのオプション一覧)の通り。また、デフォルトでは`docker-compose.yml`のbuild argsで`ASRC_BUILD_OPTIONS`として改行区切り文字列で`install_all.py`への共通オプションを指定できる。

あるいは、workコンテナの中で直接`install_all.py`を実行してもよい。むしろ試行錯誤で頻繁にリビルドする場合はそちらの方が簡便である。

## インストール方法(ホスト側で直接インストールする場合)

### Python仮想環境の起動

　本シミュレータのビルドスクリプトは"python"コマンドでインタプリタを起動する仕様となっている。
そのため、venvやconda等を用いて仮想環境を利用することを推奨する。

```sh
cd your/path/to/create/virtual/env
python3 -m venv env_name
source env_name/bin/activate
```

### setuptoolsのインストール

後述のインストール用スクリプトはビルド時にsetuptools>=62.4が必要なため、もし未インストールの場合は事前にインストールしておく。

```sh
pip install "setuptools>=62.4"
```

### シミュレータのビルド・インストール

リポジトリ直下で、

```sh
python install_all.py -b -i -c # 本体部分
python install_all.py -b -i -p # core_plugins以下に存在するプラグイン全て
python install_all.py -b -i -u # user_plugins以下に存在するプラグイン全て
#python install_all.py -b -i -a #-aは-c -p -uを全て指定するのと同じ効果
```

の順に実行すると、本体、core plugin、user pluginの順にビルド・インストールされる。
`-b`オプションはwheelをビルドするものであり、既存のwheelを使用する場合はこれを省略することでリビルドせずインストールのみを行える。
また、リビルドの際には`--clean`を付けて一時ファイルを一度削除することを推奨する。

### 深層学習用フレームワークのインストール

シミュレータ及びサンプルの全ての機能を使用するためにはPyTorchとTensorflow(Tensorboard)が必要であるため、
各ユーザーは自身の所望のバージョンのこれらのフレームワークを別途インストールしておく必要がある。
なお、PyTorchについては一部のプラグインのrequirementsに含まれているため、未インストールだった場合はpip経由で
自動的にインストールされる。

### 第5回空戦AIチャレンジ用サンプルのビルド・インストール

リポジトリ直下で、-mオプションをつけて

```sh
python install_all.py -b -i -m "./sample/modules/r8_contest_sample"
```

を実行するとr8_contest_sampleパッケージがuser pluginとしてインストールされる。

### ドキュメントの生成

#### Dockerを使用する場合

`docker-compose.yml`の`build`サービスのコンテナが手元に存在している場合、以下のコマンドによりドキュメントを[doc/html](doc/html)以下に生成できる。トップページは[doc/html/core/index.html](doc/html/core/index.html)である。

```bash
docker compose run --rm -u $(id -u):$(id -g) -v .:/$(basename $(pwd)) build bash -c "source /opt/data/venv/bin/activate && cd /$(basename $(pwd)) && bash doc/scripts/clean_docs.sh && bash doc/scripts/generate.sh"
```

#### Dockerを使用しない場合

##### Doxygenのインストール

本シミュレータはC++20の機能を使用しているため、Ubuntu 22.04のaptリポジトリから得られるDoxygenでは正しくドキュメントを生成できない。最新版のバイナリをGitHub等からダウンロードしてインストールすること。

```sh
# graphviz、pdf2svgをインストール
sudo apt-get install -y graphviz pdf2svg

# LATEXをインストール
sudo apt-get install -y texlive-lang-japanese texlive-latex-extra texlive-luatex evince

# LATEXのインストール中に、
# "Pregenerating ConTeXt MarkIV format. This may take some time..."
# というメッセージが出たところで止まってしまう場合、
# Enter を連打(長押し)することでインストールが完了する。

# Doxygenの最新版のバイナリをダウンロード
curl https://github.com/doxygen/doxygen/releases/download/Release_1_14_0/doxygen-1.14.0.linux.bin.tar.gz -o doxygen-1.14.0.linux.bin.tar.gz -L

tar -xf doxygen-1.14.0.linux.bin.tar.gz
cd doxygen-1.14.0
sudo make
sudo make install
```

##### ドキュメント生成スクリプトの実行

上記のインストールが完了した後、本リポジトリでシミュレータ及びプラグインを一度ビルドした後の状態かつ仮想環境を起動した状態で、以下を実行すると[doc/html](doc/html)以下にドキュメントが生成され、そのトップページは[doc/html/core/index.html](doc/html/core/index.html)として生成される。

```sh
bash ./doc/scripts/generate.sh
```

### install_all.pyのオプション一覧

```txt
usage: install_all.py [-h] [--clean] [-s] [-b] [-i] [-a] [-c] [-p] [-u] [-m [MANUAL_PATHS ...]] [-o] [-f [FIND_LINKS ...]] [-n] [--debug] [--msys]

options:
  -h, --help            show this help message and exit
  --clean               use when you want to clean and rebuild
  -s, --sdist           use when you want to generate sdists
  -b, --bdist_wheel     use when you want to generate wheels
  -i, --install         use when you want to install packages
  -a, --all_packages    use when you want to clean and rebuild core package (ASRCAISim1 itself) and all (core/user) plugins
  -c, --core            use when you want to clean and rebuild core package (ASRCAISim1 itself)
  -p, --core-plugins    use when you want to clean and rebuild core plugins
  -u, --user-plugins    use when you want to clean and rebuild use plugins
  -m [MANUAL_PATHS ...], --manual-paths [MANUAL_PATHS ...]
                        use when you want to designate paths manually
  -o, --offline         use when you want to build in an offline environment
  -f [FIND_LINKS ...], --find-links [FIND_LINKS ...]
                        use to give the find-links option passed to pip when you are in an offline environment
  -n, --no-build-isolation
                        use when you want not to isolate build environment
  -d, --dist-dir        path to place built wheels
  --debug               use when you want to build C++ module as debug mode
  --msys                use when you want to use MSYS
```

## 第5回空戦AIチャレンジ用サンプルの動作確認 {#page_r8_contest_getting_started}

### シミュレータ本体の動作確認 {#section_installation_check_simulator}

sample/scripts/R8Contest/MinimumEvaluationに移動し、

```sh
python evaluator.py "Rule-Fixed" "Rule-Fixed" -n 1 -v -l "./result"
```

を実行すると、オープン部門のシナリオでルールベースの初期行動判断モデルどうしの戦闘が可視化しながら1回行われ、その結果を保存したログが./resultに保存される。GUIの無い環境で使用したい場合は、-vのオプションを省略することで可視化を無効化できる。
ユース部門のシナリオで実行したい場合は-yオプションを指定する。

### HandyRLを用いた学習のサンプル {#section_installation_check_HandyRL_sample}

#### 学習の実施 {#section_installation_check_HandyRL_train}

sample/scripts/R8Contest/HandyRLSampleに移動し、

```sh
python main.py R8_contest_open_sample_M.yaml --train
```

を実行すると、オープン部門のシナリオで1体で陣営全体を操作する行動判断モデルについて、Self-Playによる学習が行われる。
学習結果は./results/Open/Multi/YYYYmmddHHMMSS以下に保存される。
なお、"M"を"S" に変更すると、1 体で1機を操作する行動判断モデルとなり、保存先は"Multi"の代わりに"Single"となる。
また、ユース部門の場合は上記のopen,Openをyouth,Youthと読み替えること。

#### 学習済モデルの評価 {#section_installation_check_HandyRL_evaluation}

学習済モデルは、./results/Open/Multi/YYYYmmddHHMMSS/policies/checkpoints以下に保存されている。
このモデルの評価は上記のevaluator.pyを使用することで可能である。
sample/scripts/R8Contest/MinimumEvaluation/candidates.jsonを開き、例えば

```json
"test":{
    "userModuleID":"HandyRLSample01M",
    "args":{"weightPath":"<学習済の.pthファイルのフルパス>"}
}
```

のように候補を追加し、sample/scripts/R8Contest/MinimumEvaluation上で

```sh
python evaluator.py "test" "Rule-Fixed" -n 10 -v -l "eval_test.csv"
```

と実行すると、初期行動判断モデルとの対戦による学習済モデルの評価を行うことができる。
ユース部門の場合は上記のopen,Openをyouth,Youthと読み替えること。また、-yオプションの指定を忘れないこと。

## アンインストール方法

pip uninstallでもよいが、プラグインを一括でアンインストールするためのスクリプトuninstall.pyを用意している。

```sh
python uninstall_all.py -u # user plugin全て
python uninstall_all.py -p # core plugin全て
python uninstall_all.py -c # 本体部分
#python uninstall_all.py -a #-aは-c -p -uを全て指定するのと同じ効果
```

### uninstall_all.pyのオプション一覧

```txt
usage: uninstall_all.py [-h] [-a] [-c] [-p] [-u]

options:
  -h, --help          show this help message and exit
  -a, --all_packages  use when you want to clean and rebuild core package (ASRCAISim1 itself) and all (core/user) plugins
  -c, --core          use when you want to clean and rebuild core package (ASRCAISim1 itself)
  -p, --core-plugins  use when you want to clean and rebuild core plugins
  -u, --user-plugins  use when you want to clean and rebuild use plugins
```
