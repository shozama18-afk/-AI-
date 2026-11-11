rem Copyright (c) 2024-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)

set prev=%cd%
set current=%~dp0
cd %current%\deps

rem ここにダウンロードしてきた依存ライブラリをビルドする処理を記述する。

rem ここにダウンロードしてきた外部ライブラリをビルドする処理を記述する。
rem その際、シミュレータ本体やプラグインがリンクした外部ライブラリ(BoostやEigen等)を
rem 使用できるようにするために、外部ライブラリのCMakeLists.txtをそのまま使用するのではなく、
rem $current/../CMakeLists.txtとして配置したラッパーファイルを用いることとする。

rem 上記のラッパーファイルを用いると、例えば外部ライブラリの名前を"ABC"としたとき、以下のような引数でcmakeコマンドを呼び出せばよい。
rem 外部ライブラリに渡すオプションは必要に応じて追記すること。
rem mkdir %BUILD_DIR%\thirdParty\proj
rem set cmake_args=^
rem  -S %PROJECT_ROOT_DIR%\thirdParty^
rem  -B %BUILD_DIR%\thirdParty\ABC^
rem  -DTHIRDPARTY_SOURCE_DIR=%PROJECT_ROOT_DIR%\thirdParty\deps\ABC^
rem  -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE%^
rem  -DCMAKE_INSTALL_PREFIX=%BUILD_DIR%\thirdParty\install^

rem if defined ASRC_CMAKE_PREFIX_PATH (
rem     if [%ASRC_CMAKE_PREFIX_PATH%] neq [] (
rem         set cmake_args=%cmake_args% -DCMAKE_PREFIX_PATH=%ASRC_CMAKE_PREFIX_PATH%
rem     )
rem )
rem cmake %cmake_args%

rem cd %BUILD_DIR%\thirdParty\ABC

rem ビルドが必要ならばmakeを呼び出す。
rem cmake --build . -j %NUMBER_OF_PROCESSORS% --config %CMAKE_BUILD_TYPE%

rem ビルド不要なもの(ヘッダオンリーライブラリ等)は直接make installを呼び出す。
rem cmake --install .
rem cd %BUILD_DIR%

cd %prev%
