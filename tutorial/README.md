# 初級セミナー チュートリアル

第5回空戦AIチャレンジの配布シミュレータを初めて触る人向けの教材である。
ルートの `README.md`(配布データの説明)と `make_agent.ipynb`(エージェント実装の説明)の要点を
1本にまとめ、**最短で「自分のエージェントで対戦する」ところまで到達できる**ようにしている。

> このチュートリアルは `atla5-simulator_dist` 本体とは別に配布する。利用する際は、展開した
> `tutorial/` ディレクトリを `atla5-simulator_dist` の直下に配置すること。本書では
> `simulator_dist/tutorial/` というディレクトリ構成を前提とする。

## 進め方

1. **事前準備**: ルートの `README.md` の「環境構築」に従って環境を用意する。
   以下が通れば準備完了。

   ```bash
   python -c "import ASRCAISim1; print('OK')"
   ```

2. **本編**: `tutorial.ipynb` を開いて上から順に進める(所要 約20分)。

3. **その後**: `SimpleAgent/` を書き換えて自分のエージェントを作る。
   強化学習で学習させたい場合は `tutorial.ipynb` の §7 を参照。

## ファイル構成

| ファイル / ディレクトリ | 内容 | セミナーで読むか |
| --- | --- | --- |
| `tutorial.ipynb` | **本体。** 20分の説明用ノートブック | ◎ |
| `SimpleAgent/` | **学習を使わない最小エージェント。** 本編 §4〜§6 で読む | ◎ |
| `SimpleAgent/__init__.py` | 提出に必須の4関数と、ルールベースの Policy | ◎ |
| `SimpleAgent/agent.py` | Agent クラス本体(必須メソッドのみ) | ◎ |
| `SimpleAgent/config.json` | Agent の設定(modelConfig) | ○ |
| `SimpleAgent/args.json` | 4関数に渡す引数。ファイル名は固定 | ○ |
| `RLAgent/` | 強化学習で学習できるエージェント一式 | §7 の導線先 |
| `MyReward.py`, `reward_config.json` | 報酬の実装例と設定 | §7 の導線先 |
| `configs/`, `seminar_config.yaml` | 学習の設定ファイル | §7 の導線先 |
| `setup_workspace.py` | 学習用の作業ディレクトリ `workspace/` を組み立てる | §7 の導線先 |

## 実行時の注意

`validate.py` はリポジトリのルートにあり、`--myagent-module-path` は
**実行時のカレントディレクトリからの相対パス**で指定する必要がある。
`tutorial/` の中から実行するのではなく、ルートに移動してから実行すること。

```bash
cd /path/to/simulator_dist
python validate.py --myagent-module-path tutorial/SimpleAgent --num-validation 3
```

## このディレクトリの位置づけ

- ルートの `make_agent.ipynb` は**網羅的な詳細リファレンス**として引き続き参照する。
  `control` / `behave` / `perceive` / `serializeInternalState` や M方式(中央集権方式)の
  書き方はそちらに載っている。
- 仕様で迷ったら `docs/html/core/index.html`(Doxygen ドキュメント)を開く。
