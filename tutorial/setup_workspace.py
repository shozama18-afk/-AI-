#!/usr/bin/env python
"""学習用の作業ディレクトリ tutorial/workspace/ を組み立てる。

強化学習サンプル(HandyRL)は
`simulator/sample/scripts/R8Contest/HandyRLSample/` に同梱されている。
学習を行うにはその一式と、自作のエージェント・報酬・設定ファイルが
同じディレクトリに揃っている必要があるため、このスクリプトで集約する。

    python tutorial/setup_workspace.py
    cd tutorial/workspace
    python main.py seminar_config.yaml --train

【重要】このスクリプトは tutorial/workspace/ 以下にしか書き込まない。
配布物の他のファイル(simulator/ 以下など)は読み取るだけで変更しない。

--clean を付けると workspace/ を作り直す(学習結果 results/ も消えるので注意)。
"""

import argparse
import json
import os
import shutil
import sys

# ----------------------------------------------------------------------
# パス
# ----------------------------------------------------------------------
TUTORIAL_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(TUTORIAL_DIR)
HANDYRL_DIR = os.path.join(
    ROOT_DIR, "simulator", "sample", "scripts", "R8Contest", "HandyRLSample")
R8_SAMPLE_DIR = os.path.join(
    ROOT_DIR, "simulator", "sample", "modules", "r8_contest_sample", "r8_contest_sample")
WORKSPACE_DIR = os.path.join(TUTORIAL_DIR, "workspace")

# r8_contest_sample のうち C++ に依存しない純Pythonファイル。
# パッケージの __init__.py は common_init_for_plugin で C++ 拡張を読み込むため、
# それを経由せずに済むよう、これらだけを別パッケージ名でコピーして使う。
# こうすることで r8_contest_sample をビルドしなくても学習・推論ができる。
PY_TWIN_PACKAGE = "r8_sample_py"
PY_TWIN_FILES = [
    "R8ContestTorchNNSampleForHandyRL.py",  # NN 本体
    "GenericTorchModelUtil.py",             # NN の部品
    "SetTransformer.py",                    # NN の部品
    "R8ContestPyRewardSample01.py",         # 公式 shaping 報酬(TeamReward)
    "R8ContestPyRewardSample02.py",         # 公式 個別報酬(AgentReward)
    "R8ContestPyAgentSample01.py",          # 公式 Agent サンプル(参考用)
    "PyVirtualSimulatorSample.py",          # 上記 Agent の部品
]

PY_TWIN_INIT = '''"""r8_contest_sample の純Pythonクラスだけを取り出したパッケージ。

tutorial/setup_workspace.py が
simulator/sample/modules/r8_contest_sample/r8_contest_sample/
からコピーして生成する。直接編集しても再実行すると上書きされる。

本来のパッケージ r8_contest_sample は __init__.py で C++ 拡張を読み込むため
ビルドが必要になるが、ここに置いたファイルはいずれも
ASRCAISim1.core / BasicAgentUtility / torch / gymnasium にしか依存しないため、
ビルドなしで利用できる。
"""
'''

IGNORE = shutil.ignore_patterns("__pycache__", "*.pyc")

# HandyRLSample から持ってくる設定ファイル
CONFIG_FILES = [
    "R8_contest_open_mission_config.json",
    "R8_contest_open_asset_placeholder.json",
    "R8_contest_youth_mission_config.json",
    "R8_contest_youth_asset_placeholder.json",
    "R8_contest_agent_ruler_reward_models.json",
]

# tutorial/ から持ってくる教材ファイル
TUTORIAL_FILES = [
    "seminar_config.yaml",
    "MyReward.py",
    "reward_config.json",
]

# ----------------------------------------------------------------------
# SampleEnv/sample.py の生成
#
# 学習時に自作の Agent と Reward を Factory へ登録する必要がある。
# 配布されている handyrl/envs/ASRC/ASRCAISim1.py にその登録処理を挿入したものを
# handyrl/envs/SampleEnv/sample.py として生成する。
#
# 配布物をコピーした後の workspace/ 内のファイルを加工するので、
# 配布物そのものは変更されない。また元ファイルから毎回生成するため、
# 配布物側が更新されても追従する。
# ----------------------------------------------------------------------
IMPORT_ANCHOR = "import collections"
IMPORT_ADDITION = """import json
import importlib"""

CORE_IMPORT_ANCHOR = "from ASRCAISim1.core import ExpertWrapper, SimpleMultiPortCombiner"
CORE_IMPORT_ADDITION = "from ASRCAISim1.common import addPythonClass"

REGISTER_ANCHOR = 'original_overrider=args["env_config"].get("overrider",lambda c,w,v:c)'
REGISTER_BLOCK = '''
        # ==============================================================
        # ここから: 自作の Agent と Reward を Factory へ登録する
        # (tutorial/setup_workspace.py が挿入したブロック)
        #
        # Environment は Worker ごとにインスタンス化されるため、
        # すべてのインスタンス上で独立に登録が行われる必要がある。
        # ==============================================================

        # --- Agent の登録 ---
        # yaml の env_args.userModelID / userModuleID を見て、
        # "Agent_{userModelID}" というクラス名で Factory に登録する。
        # この名前を学習設定ファイルの AgentConfigDispatcher の
        # "Learner_e" -> "model" と一致させることで学習対象になる。
        userModelID = args["userModelID"]
        userModuleID = args["userModuleID"]
        with open(os.path.join(userModuleID, args["modelargs"])) as f:
            model_args = json.load(f)

        module = importlib.import_module(userModuleID)
        assert hasattr(module, "getUserAgentClass")
        assert hasattr(module, "getUserAgentModelConfig")
        assert hasattr(module, "isUserAgentSingleAsset")
        assert hasattr(module, "getUserPolicy")

        agentClass = module.getUserAgentClass(model_args)
        addPythonClass("Agent", "Agent_" + userModelID, agentClass)
        Factory.addDefaultModel(
            "Agent",
            "Agent_" + userModelID,
            {
                "class": "Agent_" + userModelID,
                "config": module.getUserAgentModelConfig(model_args)
            }
        )

        # --- 公式サンプルの報酬(純Python版)の登録 ---
        # C++ をビルドしなくても使えるよう、r8_sample_py にコピーしたものを使う。
        # Factory のモデル定義は configs/seminar_reward_models.json 側にある。
        from r8_sample_py.R8ContestPyRewardSample01 import R8ContestPyRewardSample01
        from r8_sample_py.R8ContestPyRewardSample02 import R8ContestPyRewardSample02
        addPythonClass("Reward", "R8ContestPyRewardSample01", R8ContestPyRewardSample01)
        addPythonClass("Reward", "R8ContestPyRewardSample02", R8ContestPyRewardSample02)

        # --- Reward の登録 ---
        # yaml の env_args.userRewardModuleID のモジュールから MyReward クラスを
        # 読み込んで登録する。この実装ではクラス名は MyReward で固定である。
        # 実際に使うかどうかは学習設定ファイルの Manager.Rewards のリスト次第。
        userRewardModuleID = args["userRewardModuleID"]
        with open(args["rewardConfig"]) as f:
            reward_config = json.load(f)

        reward_module = importlib.import_module(userRewardModuleID)
        addPythonClass("Reward", userRewardModuleID, reward_module.MyReward)
        Factory.addDefaultModel(
            "Reward",
            userRewardModuleID,
            {
                "class": "MyReward",
                "config": reward_config
            }
        )
        # ==============================================================
        # ここまで
        # ==============================================================
'''


def generate_sample_env(workspace_dir):
    """workspace/handyrl/envs/SampleEnv/sample.py を生成する。"""
    src = os.path.join(workspace_dir, "handyrl", "envs", "ASRC", "ASRCAISim1.py")
    dst_dir = os.path.join(workspace_dir, "handyrl", "envs", "SampleEnv")
    dst = os.path.join(dst_dir, "sample.py")

    with open(src, encoding="utf-8") as f:
        text = f.read()

    # 1) import の追加
    if IMPORT_ANCHOR not in text:
        raise RuntimeError(
            f"アンカーが見つかりません: {IMPORT_ANCHOR!r}\n"
            f"  {src} の構成が変わった可能性があります。")
    text = text.replace(IMPORT_ANCHOR, IMPORT_ANCHOR + "\n" + IMPORT_ADDITION, 1)

    # 2) Factory と addPythonClass の import
    idx = text.find(CORE_IMPORT_ANCHOR)
    if idx < 0:
        raise RuntimeError(
            f"アンカーが見つかりません: {CORE_IMPORT_ANCHOR!r}\n"
            f"  {src} の構成が変わった可能性があります。")
    eol = text.index("\n", idx)
    line = text[idx:eol]
    if "Factory" not in line:
        line = line + ", Factory"
    text = text[:idx] + line + "\n" + CORE_IMPORT_ADDITION + text[eol:]

    # 3) 登録処理の挿入
    idx = text.find(REGISTER_ANCHOR)
    if idx < 0:
        raise RuntimeError(
            f"アンカーが見つかりません: {REGISTER_ANCHOR!r}\n"
            f"  {src} の構成が変わった可能性があります。")
    eol = text.index("\n", idx)
    text = text[:eol + 1] + REGISTER_BLOCK + text[eol + 1:]

    header = (
        '"""自作の Agent と Reward を Factory へ登録する学習用 Environment。\n'
        "\n"
        "このファイルは tutorial/setup_workspace.py が\n"
        "handyrl/envs/ASRC/ASRCAISim1.py から自動生成したものである。\n"
        "直接編集しても setup_workspace.py を再実行すると上書きされる。\n"
        '"""\n'
    )

    os.makedirs(dst_dir, exist_ok=True)
    with open(dst, "w", encoding="utf-8", newline="\n") as f:
        f.write(header + text)
    return dst


def install_python_twins(dest_dir):
    """r8_contest_sample の純Pythonファイルを dest_dir/r8_sample_py/ にコピーする。"""
    pkg_dir = os.path.join(dest_dir, PY_TWIN_PACKAGE)
    if os.path.isdir(pkg_dir):
        shutil.rmtree(pkg_dir)
    os.makedirs(pkg_dir)
    with open(os.path.join(pkg_dir, "__init__.py"), "w", encoding="utf-8",
              newline="\n") as f:
        f.write(PY_TWIN_INIT)
    missing = []
    for name in PY_TWIN_FILES:
        src = os.path.join(R8_SAMPLE_DIR, name)
        if os.path.exists(src):
            shutil.copy2(src, os.path.join(pkg_dir, name))
        else:
            missing.append(name)
    if missing:
        raise RuntimeError(
            "r8_contest_sample の純Pythonファイルが見つかりません: "
            + ", ".join(missing)
            + f"\n  探した場所: {R8_SAMPLE_DIR}")
    return pkg_dir


NN_IMPORT_OLD = "from r8_contest_sample.R8ContestTorchNNSampleForHandyRL import"
NN_IMPORT_NEW = f"from {PY_TWIN_PACKAGE}.R8ContestTorchNNSampleForHandyRL import"

# ray._private.ray_constants.NODE_DEFAULT_IP は ray の内部APIであり、
# バージョンによっては存在しない(配布環境のピンは dist/*/requirements.txt の ray==2.46.0)。
# main.py ではこの定数を entrypoint_ip_address の既定値としてしか使っておらず、
# seminar_config.yaml は明示的に 127.0.0.1 を指定しているため実際には使われない。
# 別バージョンの ray を入れた環境でも動くようフォールバックを入れる。
RAY_IMPORT_OLD = "from ray._private.ray_constants import NODE_DEFAULT_IP"
RAY_IMPORT_NEW = """try:
    from ray._private.ray_constants import NODE_DEFAULT_IP
except ImportError:
    # ray の内部APIで、バージョンによっては存在しない。既定値としてしか使われないため実害はない。
    NODE_DEFAULT_IP = "127.0.0.1\""""


def patch_main_py(workspace_dir):
    """workspace/main.py の NN の import を、コピーした純Python版に向ける。

    配布物の main.py は r8_contest_sample パッケージから import しているが、
    そのパッケージは C++ のビルドが必要なため、ビルド不要な複製を使うよう書き換える。
    書き換えるのは workspace/ にコピーした側だけで、配布物は変更しない。
    """
    path = os.path.join(workspace_dir, "main.py")
    with open(path, encoding="utf-8") as f:
        text = f.read()
    for old in (NN_IMPORT_OLD, RAY_IMPORT_OLD):
        if old not in text:
            raise RuntimeError(
                f"アンカーが見つかりません: {old!r}\n"
                f"  {path} の構成が変わった可能性があります。")
    text = text.replace(NN_IMPORT_OLD, NN_IMPORT_NEW)
    text = text.replace(RAY_IMPORT_OLD, RAY_IMPORT_NEW)
    header = (
        "# 注) このファイルは tutorial/setup_workspace.py がコピーし、以下を書き換えたものである。\n"
        "#       - NN の import 先を r8_sample_py(ビルド不要な複製)に変更\n"
        "#       - ray のバージョンによっては存在しない NODE_DEFAULT_IP にフォールバックを追加\n"
        "#     元ファイル: simulator/sample/scripts/R8Contest/HandyRLSample/main.py\n"
    )
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(header + text)


def check_model_config_consistency():
    """学習時と推論時のネットワーク構成が一致しているかを検査する。

    食い違っていると学習済み重み(.pth)の読み込みに失敗するため、
    ここで気付けるようにしておく。
    """
    try:
        import yaml
    except ImportError:
        print("  warning: PyYAML が無いため model_config の整合検査をスキップした")
        return True

    train_path = os.path.join(TUTORIAL_DIR, "seminar_config.yaml")
    infer_path = os.path.join(TUTORIAL_DIR, "RLAgent", "model_config.yaml")
    if not (os.path.exists(train_path) and os.path.exists(infer_path)):
        return True

    with open(train_path, encoding="utf-8") as f:
        train = yaml.safe_load(f)["policy_config"]["Learner"]["model_config"]
    with open(infer_path, encoding="utf-8") as f:
        infer = yaml.safe_load(f)

    if json.dumps(train, sort_keys=True) == json.dumps(infer, sort_keys=True):
        return True

    print()
    print("!" * 70)
    print("警告: 学習時と推論時のネットワーク構成が一致していない")
    print("  学習時: seminar_config.yaml -> policy_config -> Learner -> model_config")
    print("  推論時: RLAgent/model_config.yaml")
    only_train = sorted(set(train) - set(infer))
    only_infer = sorted(set(infer) - set(train))
    differ = sorted(k for k in set(train) & set(infer)
                    if json.dumps(train[k], sort_keys=True)
                    != json.dumps(infer[k], sort_keys=True))
    if only_train:
        print(f"  学習時にのみ存在するキー: {only_train}")
    if only_infer:
        print(f"  推論時にのみ存在するキー: {only_infer}")
    if differ:
        print(f"  内容が異なるキー        : {differ}")
    print()
    print("このままでは学習済み重みの読み込みに失敗する可能性がある。")
    print("次のコマンドで RLAgent/model_config.yaml を作り直すこと。")
    print()
    print('    python -c "import yaml; '
          "c=yaml.safe_load(open('tutorial/seminar_config.yaml',encoding='utf-8')); "
          "yaml.safe_dump(c['policy_config']['Learner']['model_config'], "
          "open('tutorial/RLAgent/model_config.yaml','w',encoding='utf-8'), "
          'allow_unicode=True, sort_keys=True, indent=4)"')
    print("!" * 70)
    print()
    return False


# ----------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clean", action="store_true",
                        help="workspace/ を削除してから作り直す")
    opts = parser.parse_args()

    if not os.path.isdir(HANDYRL_DIR):
        print(f"ERROR: 強化学習サンプルが見つかりません: {HANDYRL_DIR}", file=sys.stderr)
        return 1

    if opts.clean and os.path.isdir(WORKSPACE_DIR):
        print(f"removing {os.path.relpath(WORKSPACE_DIR, ROOT_DIR)}")
        shutil.rmtree(WORKSPACE_DIR)

    os.makedirs(WORKSPACE_DIR, exist_ok=True)
    created = []

    # --- handyrl 一式 ---
    dst = os.path.join(WORKSPACE_DIR, "handyrl")
    if os.path.isdir(dst):
        shutil.rmtree(dst)
    shutil.copytree(os.path.join(HANDYRL_DIR, "handyrl"), dst, ignore=IGNORE)
    created.append("handyrl/")

    # --- main.py ---
    shutil.copy2(os.path.join(HANDYRL_DIR, "main.py"),
                 os.path.join(WORKSPACE_DIR, "main.py"))
    created.append("main.py")

    # --- 設定ファイル ---
    configs_dst = os.path.join(WORKSPACE_DIR, "configs")
    os.makedirs(configs_dst, exist_ok=True)
    for name in CONFIG_FILES:
        src = os.path.join(HANDYRL_DIR, "configs", name)
        if os.path.exists(src):
            shutil.copy2(src, os.path.join(configs_dst, name))
            created.append(f"configs/{name}")
        else:
            print(f"  warning: not found, skipped: {src}")

    # --- 教材側の学習設定 ---
    for name in os.listdir(os.path.join(TUTORIAL_DIR, "configs")):
        if not name.endswith(".json"):
            continue
        shutil.copy2(os.path.join(TUTORIAL_DIR, "configs", name),
                     os.path.join(configs_dst, name))
        created.append(f"configs/{name}")

    # --- r8_contest_sample の純Python版(ビルド不要にするため) ---
    # 推論時(validate.py / 提出物)にも NN が必要なので RLAgent 側にも置く。
    # workspace へのコピーより先に行うことで、workspace/RLAgent にも入る。
    install_python_twins(os.path.join(TUTORIAL_DIR, "RLAgent"))
    created.append(f"../RLAgent/{PY_TWIN_PACKAGE}/ (r8_contest_sample の純Python版をコピー)")

    # --- エージェント ---
    dst = os.path.join(WORKSPACE_DIR, "RLAgent")
    if os.path.isdir(dst):
        shutil.rmtree(dst)
    shutil.copytree(os.path.join(TUTORIAL_DIR, "RLAgent"), dst, ignore=IGNORE)
    created.append("RLAgent/")

    # --- 報酬と学習設定 ---
    for name in TUTORIAL_FILES:
        shutil.copy2(os.path.join(TUTORIAL_DIR, name),
                     os.path.join(WORKSPACE_DIR, name))
        created.append(name)

    # --- 学習側(main.py / sample.py)からも使えるよう workspace 直下にも置く ---
    install_python_twins(WORKSPACE_DIR)
    created.append(f"{PY_TWIN_PACKAGE}/ (同上)")

    # --- main.py の import を書き換え ---
    patch_main_py(WORKSPACE_DIR)
    created.append("main.py (NN の import 先を書き換え)")

    # --- SampleEnv/sample.py の生成 ---
    generate_sample_env(WORKSPACE_DIR)
    created.append("handyrl/envs/SampleEnv/sample.py (自動生成)")

    # --- 結果 ---
    print(f"workspace: {WORKSPACE_DIR}")
    for name in created:
        print(f"  + {name}")
    print()

    # --- 学習時と推論時のネットワーク構成の整合検査 ---
    check_model_config_consistency()

    print("次のコマンドで学習を開始できる:")
    print(f"    cd {os.path.relpath(WORKSPACE_DIR, os.getcwd())}")
    print("    python main.py seminar_config.yaml --train")
    print()
    print("学習済みモデルは results/Open/Single/<日時>/policies/checkpoints/ に保存される。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
