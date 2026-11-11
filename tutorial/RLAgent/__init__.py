"""提出フォーマットの実体 (S方式: 1機に1エージェント / 強化学習版)

SimpleAgent との違いは getUserPolicy だけである。
ルールベースの Policy ではなく、学習済みの重み(.pth)を読み込んで
ニューラルネットワークで行動を決める StandaloneHandyRLPolicy を返す。

重みファイルの探し方は次の順である。
  1. args.json の "weightPath" で指定されたファイル
  2. このディレクトリ直下の *.pth のうち最初に見つかったもの
  3. 見つからなければ警告を出して未学習(ランダム初期値)のまま動く
"""

import glob
import json
import os

import yaml

from ASRCAISim1.plugins.HandyRLUtility.StandaloneHandyRLPolicy import StandaloneHandyRLPolicy  # type: ignore
from ASRCAISim1.plugins.HandyRLUtility.distribution import getActionDistributionClass  # type: ignore


def getUserAgentClass(args={}):
    """使用する Agent クラスを返す。"""
    from .agent import RLAgent
    return RLAgent


def getUserAgentModelConfig(args={}):
    """Agent に渡す設定(modelConfig)を返す。"""
    with open(os.path.join(os.path.dirname(__file__), "agent_config.json"), "r") as f:
        return json.load(f)


def isUserAgentSingleAsset(args={}):
    """1機に1エージェントを割り当てる S方式 なので True。

    M方式(編隊全体を1エージェントで操作)にする場合は False にしたうえで、
    agent_config.json の maxParentNum を 4、maxFriendNum を 1 にすること。
    """
    return True


def getUserPolicy(args={}):
    """学習済みモデルで行動を決める Policy を返す。"""
    # ニューラルネットワーク本体は r8_contest_sample にある。
    # ただしそのパッケージは __init__.py で C++ 拡張を読み込むためビルドが必要なので、
    # 入っていない場合は tutorial/setup_workspace.py がコピーした純Python版を使う。
    # (NN のファイル自体は torch と HandyRLUtility にしか依存しない)
    try:
        from r8_contest_sample.R8ContestTorchNNSampleForHandyRL import (
            R8ContestTorchNNSampleForHandyRL,
        )
    except ImportError:
        from .r8_sample_py.R8ContestTorchNNSampleForHandyRL import (
            R8ContestTorchNNSampleForHandyRL,
        )

    here = os.path.dirname(__file__)

    # 学習時に使用した model_config.yaml と同等のものである必要がある
    # (seminar_config.yaml の policy_config -> Learner -> model_config と整合させること)
    with open(os.path.join(here, "model_config.yaml"), "r") as f:
        model_config = yaml.safe_load(f)

    weightPath = None
    if args is not None:
        weightPath = args.get("weightPath", None)
    if weightPath is not None:
        weightPath = os.path.join(here, weightPath)
        if not os.path.exists(weightPath):
            weightPath = None
    if weightPath is None:
        candidates = glob.glob(os.path.join(here, "*.pth"))
        weightPath = candidates[0] if len(candidates) > 0 else None
    if weightPath is None:
        print("Warning: Model weight file was not found. ", __name__)

    # True にすると決定論的に、False にすると確率論的に行動する
    isDeterministic = False

    return StandaloneHandyRLPolicy(
        R8ContestTorchNNSampleForHandyRL,
        model_config,
        weightPath,
        getActionDistributionClass,
        isDeterministic,
    )
