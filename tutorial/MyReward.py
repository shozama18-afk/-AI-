"""報酬の実装例

強化学習では「何を評価するか」を報酬として与える必要がある。
本シミュレータでは報酬も登場物の一種として実装する。

## 2種類の基底クラス

| 基底クラス | 対象 | self.reward のキー |
| --- | --- | --- |
| `TeamReward` | 陣営全体で共有する報酬 | 陣営名("Red" / "Blue") |
| `AgentReward` | Agent ごとに個別に与える報酬 | Agent の full name |

## 呼ばれるタイミング

| メソッド | タイミング | 備考 |
| --- | --- | --- |
| `onEpisodeBegin` | エピソード開始時 | 基底クラスが config に基づく対象設定を行う |
| `onStepBegin` | step 開始時 | 基底クラスが step 報酬を 0 にリセットする |
| `onInnerStepBegin` | インナーステップ開始時 | 既定では何もしない |
| `onInnerStepEnd` | インナーステップ終了時 | **一定周期で呼ばれるので、極力ここで計算する** |

オーバーライドする場合は基底クラスの処理(`super()`)を呼ぶか、同等の処理を書くこと。

## 登録のしかた

このファイルの `MyReward` クラスは、学習環境の構築時に Factory へ動的に登録される
(`handyrl/envs/SampleEnv/sample.py` が `seminar_config.yaml` の `env_args` ->
`userRewardModuleID` を見て import する)。**この仕組みではクラス名を `MyReward` に
固定する必要がある。**

実際に使われるかどうかは学習設定ファイル
`configs/seminar_learning_config_S.json` の `Manager` -> `Rewards` のリスト次第である。
リストに並べた報酬の**合計値**が実際の報酬になる。既定では以下の4つを使っている。

```json
"Rewards": [
    {"model": "MyReward",         "target": "All"},
    {"model": "SeminarShaping01", "target": "All"},
    {"model": "SeminarShaping02", "target": "All"},
    {"model": "MyWinLoseReward",  "target": "All"}
]
```

- `MyReward` …… このファイルの実装。学習環境の構築時に Factory へ動的に登録される。
- `SeminarShaping01` / `SeminarShaping02` …… 後述の公式サンプル報酬(純Python版)。
  Factory のモデル定義は `configs/seminar_reward_models.json` にある。
- `MyWinLoseReward` …… 勝敗そのものに対する報酬。
  Factory のモデル定義は `configs/R8_contest_agent_ruler_reward_models.json` にある。

自分の報酬だけで学習させたい場合は、リストから不要な行を消せばよい。
`target` を `"All"` にすると、場に存在するすべての陣営および Agent が計算対象になる。

## もっと作り込むには

`r8_contest_sample` に公式の実装例がある。何を評価しているかを読むと参考になる。

- `R8ContestPyRewardSample01.py` (`TeamReward`) …… 探知の維持、彼機の探知率、過大な機動、
  前進・後退、力学的エネルギーの変化などを評価する shaping 報酬
- `R8ContestPyRewardSample02.py` (`AgentReward`) …… 誘導弾の回避、被撃墜、墜落、撃墜、
  場外、終了時の残存、燃料切れなどを評価する個別報酬
"""

from ASRCAISim1.core import TeamReward, nljson, Fighter  # type: ignore


class MyReward(TeamReward):
    """陣営全体で共有する報酬。生き残っている戦闘機の数に応じて加点する。

    これは最小限の例である。実際には「勝つために何が良い行動なのか」を表す指標を
    考えて設計すること。
    """

    def __init__(self, modelConfig: nljson, instanceConfig: nljson):
        super().__init__(modelConfig, instanceConfig)
        if self.isDummy:
            # Factory によるダミー生成のため、空引数でのインスタンス化に対応させる
            return

        # reward_config.json の内容が modelConfig として渡される
        self.aliveBonus = self.modelConfig().get("aliveBonus", 0.1)

    def onEpisodeBegin(self):
        """エピソード開始時の処理。

        基底クラスで config に基づく報酬計算対象の設定等が行われるため、
        それ以外の追加処理や設定の上書きを行いたい場合のみオーバーライドする。
        """
        super().onEpisodeBegin()

    def onStepBegin(self):
        """step 開始時の処理。

        基底クラスにおいて reward(step 報酬)を 0 にリセットしているため、
        オーバーライドする場合は基底クラスの処理を呼び出す必要がある。
        """
        super().onStepBegin()

    def onInnerStepBegin(self):
        """インナーステップ開始時の処理。より細かい報酬計算が必要な場合に使う。"""
        pass

    def onInnerStepEnd(self):
        """インナーステップ終了時の処理。一定周期で呼ばれるのでここで計算する。"""
        for team in self.reward:
            # team に属している Asset(Fighter)を取得する
            for f in self.manager.getAssets(
                    lambda a: a.getTeam() == team and isinstance(a, Fighter)):
                if f().isAlive():
                    self.reward[team] += self.aliveBonus
