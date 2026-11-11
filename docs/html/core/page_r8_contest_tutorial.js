var page_r8_contest_tutorial =
[
    [ "第5回空戦AIチャレンジの戦闘場面", "page_r8_contest_scenario.html", [
      [ "場面設定の概要", "page_r8_contest_scenario.html#section_r8_contest_scenario_overview", [
        [ "シミュレーション及び行動判断の周期", "page_r8_contest_scenario.html#section_r8_contest_scenario_tick", null ],
        [ "戦闘空間の定義", "page_r8_contest_scenario.html#section_r8_contest_scenario_combat_area", null ],
        [ "戦闘のルール", "page_r8_contest_scenario.html#section_r8_contest_scenario_combat_rule", [
          [ "終了条件", "page_r8_contest_scenario.html#section_r8_contest_scenario_combat_termination", null ],
          [ "ペナルティ", "page_r8_contest_scenario.html#section_r8_contest_scenario_penalty", null ],
          [ "戦闘機の初期配置", "page_r8_contest_scenario.html#section_r8_contest_scenario_initial_state_of_fighters", null ],
          [ "護衛対象機の初期配置と行動", "page_r8_contest_scenario.html#section_r8_contest_scenario_escorted_aircraft", null ],
          [ "戦闘機の機体性能のランダム化", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_spec", null ]
        ] ],
        [ "航空機のモデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_model", [
          [ "運動・飛行制御モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_flight_dynamics_and_control", [
            [ "運動モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_flight_dynamics", null ]
          ] ],
          [ "飛行制御モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_flight_control", null ],
          [ "センサモデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_sensor", [
            [ "相手側航空機の探知（レーダ）", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_radar", null ],
            [ "相手側航空機の探知（IRST）", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_IRST", null ],
            [ "相手側誘導弾の探知（MWS）", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_mws", null ]
          ] ],
          [ "ネットワークによる情報共有", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_datalink", null ],
          [ "人間による介入の模擬", "page_r8_contest_scenario.html#section_r8_contest_scenario_fighter_human_intervention", null ],
          [ "武装モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_weapon", null ]
        ] ],
        [ "誘導弾のモデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_model", [
          [ "運動・飛行制御モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_flight_dynamics_and_control", [
            [ "空気力モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_aerodynamics", null ],
            [ "推力モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_flight_propulsion", null ],
            [ "運動モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_flight_dynamics", null ],
            [ "飛行制御モデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_flight_control", null ]
          ] ],
          [ "最大飛翔時間及び命中判定", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_termination", null ],
          [ "センサモデル", "page_r8_contest_scenario.html#section_r8_contest_scenario_missile_sensor", null ]
        ] ]
      ] ],
      [ "設定値の一覧", "page_r8_contest_scenario.html#section_r8_contest_scenario_parameters", null ],
      [ "参考文献", "page_r8_contest_scenario.html#section_r8_contest_scenario_citation", null ]
    ] ],
    [ "第5回空戦AIチャレンジ向けのMatchMaker", "page_r8_contest_match_maker.html", [
      [ "configの追加要素", "page_r8_contest_match_maker.html#section_r8_contest_match_maker_config", null ],
      [ "機体性能のランダム化(オープン部門向け)", "page_r8_contest_match_maker.html#section_r8_contest_match_maker_randomization", null ],
      [ "護衛対象機を含めたManager初期配置の設定", "page_r8_contest_match_maker.html#section_r8_contest_match_maker_initial_state", null ]
    ] ],
    [ "Agent が入出力 observables と commands の形式", "page_r8_contest_agent_observables_and_commands.html", [
      [ "Agent が受け取ることのできる observables", "page_r8_contest_agent_observables_and_commands.html#section_r8_contest_agent_observables", [
        [ "誘導弾に関するobservables", "page_r8_contest_agent_observables_and_commands.html#section_r8_contest_missile_observables", null ]
      ] ],
      [ "Agent が出力すべき commands", "page_r8_contest_agent_observables_and_commands.html#section_r8_contest_agent_commands", null ],
      [ "Agent が アクセス可能な Accessor", "page_r8_contest_agent_observables_and_commands.html#section_r8_contest_agent_accessible_accessors", null ]
    ] ],
    [ "ルールベースの初期行動判断モデル", "page_r8_contest_initial_rulebased_agent.html", [
      [ "航跡に対する付帯情報", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_trackinfo", null ],
      [ "目標選択", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_target_selection", null ],
      [ "行動の種類", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_state", null ],
      [ "(s1)通常時の行動", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_normal_state", null ],
      [ "(a1)射撃", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_shoot", null ],
      [ "(s2)離脱", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_withdrawal", null ],
      [ "(s3)回避", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_evasion", null ],
      [ "針路の補正", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_route_correction", null ],
      [ "目標進行方向及び目標速度の計算方法", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_command_calculation", [
        [ "方向指示", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_command_direction", null ],
        [ "速度指示", "page_r8_contest_initial_rulebased_agent.html#r8_contest_initial_rulebased_agent_command_velocity", null ]
      ] ]
    ] ],
    [ "第5回空戦AIチャレンジ向け強化学習 Agent サンプル", "page_r8_contest_agent_sample.html", [
      [ "実装の概要", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_overview", [
        [ "observablesの抽出", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_extract_observables", null ],
        [ "observationの形式", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_observation", null ],
        [ "actionの形式", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_action", null ],
        [ "行動制限について", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_action_limit", null ],
        [ "メンバ変数", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_member_variables", [
          [ "modelConfigで設定するもの", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_member_variables_from_modelConfig", null ],
          [ "内部変数", "page_r8_contest_agent_sample.html#section_r8_contest_agent_sample_member_variables_internal_state", null ]
        ] ]
      ] ]
    ] ],
    [ "第5回空戦AIチャレンジ向け強化学習 Reward サンプル", "page_r8_contest_reward_sample.html", [
      [ "サンプルその1の概要", "page_r8_contest_reward_sample.html#section_r8_contest_reward_sample_01_overview", null ],
      [ "サンプルその2の概要", "page_r8_contest_reward_sample.html#section_r8_contest_reward_sample_02_overview", null ]
    ] ],
    [ "HandyRL(の改変版)を用いた強化学習サンプル", "page_r8_contest_handyrl_sample.html", [
      [ "元の HandyRL に対する改変・機能追加の概要", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_modification", null ],
      [ "学習の実行方法", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_run_train", null ],
      [ "学習済モデルの評価", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_run_evaluation", null ],
      [ "yaml の記述方法", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_yaml_format", null ],
      [ "yaml で定義可能なニューラルネットワークのサンプル", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_nn", null ],
      [ "カスタムクラスの使用", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_custom_classes", null ],
      [ "学習ログの構成", "page_r8_contest_handyrl_sample.html#section_r8_contest_handyrl_sample_logging", null ]
    ] ],
    [ "異なる行動判断モデル同士を対戦させるための機能", "page_r8_contest_agent_as_package_and_minimum_evaluation.html", [
      [ "Agent 及び Policy のパッケージ化の方法", "page_r8_contest_agent_as_package_and_minimum_evaluation.html#section_r8_contest_agent_as_package", [
        [ "サンプルプラグインを改変していた場合のパッケージ化", "page_r8_contest_agent_as_package_and_minimum_evaluation.html#section_r8_contest_agent_as_package_with_modified_sample", null ],
        [ "C++を使用した独自プラグインを作成して使用する場合の推奨事項", "page_r8_contest_agent_as_package_and_minimum_evaluation.html#section_r8_contest_custom_cpp_plugin", null ]
      ] ],
      [ "パッケージ化された Agent 及び Policy の組を読み込んで対戦させるサンプル", "page_r8_contest_agent_as_package_and_minimum_evaluation.html#section_r8_contest_minimum_evaluation", null ]
    ] ]
];