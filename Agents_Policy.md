
モデルデータは/simulator_dist/prototype/~　に設置

agent.py/makeobs で参照,獲得するデータは/simulator_dist/docs/html/core/page_r8_contest_agent_observables_and_commands.htmlに早見表アリ
→--replay 1で.datの戦闘記録を出力する場合、なんかsimulator_dist/result/0/に入れたまんまだと今までの奴全部作り直してクソほど時間かかるので、必ずsimulator_dist/wanna_mp4/0に移動してからmp4を生成する。

→wsl
cd ~/simulator_dist
docker compose up -d,
docker exec -it atla5 bash
のコンボでdocker環境起動
python validate.py --num-validation N --youth 1 --replay 0 または 1 --myagent-module-path 「Agents/prototype/エージェント名」の呪文で戦闘実行
xvfb-run python recruit.py --movie-dir "./wanna_mp4"の呪文で/wanna_mp4のdatファイルをmp4へ変換

基本的に変更はAgents/prototype/~以下のこれらファイルに留めること。
・agent.py:エージェントの低級行動判断、情報取得を司る
・__init__.py:エージェントの高級行動判断、戦略要素を司る
・config.json,args.json:各種パラメータの値を司る

細かいことは/simulator_dist/tutorial/tutorial.ipynbを参照