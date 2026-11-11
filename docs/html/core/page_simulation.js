var page_simulation =
[
    [ "シミュレーション登場物について", "page_simulation_entity.html", "page_simulation_entity" ],
    [ "シミュレーションの処理の流れについて", "page_simulation_flow.html", [
      [ "シミュレーションの処理の流れ", "page_simulation_flow.html#section_simulation_flow", null ],
      [ "非周期的なイベントの発生及びイベントハンドラ", "page_simulation_flow.html#section_simulation_event_handler", null ]
    ] ],
    [ "シミュレータの生成及び外部インターフェース", "page_simulation_execution_interface.html", [
      [ "SimulationManager クラス", "page_simulation_execution_interface.html#section_simulation_execution_SimulationManager", [
        [ "コンストラクタ", "page_simulation_execution_interface.html#section_simulation_execution_SimulationManager_constructor", null ],
        [ "コンフィグの書式", "page_simulation_execution_interface.html#section_simulation_execution_SimulationManager_config_format", null ],
        [ "PhysiacalAsset の生成について", "page_simulation_execution_interface.html#section_simulation_execution_asset_config_dispatch", null ],
        [ "Agent の生成について", "page_simulation_execution_interface.html#section_simulation_execution_agent_config_dispatch", null ]
      ] ],
      [ "gymnasium インターフェース", "page_simulation_execution_interface.html#section_simulation_execution_as_gymnasium_env", [
        [ "GymManager クラス", "page_simulation_execution_interface.html#section_simulation_execution_GymManager", [
          [ "コンストラクタ(<b>init</b>)", "page_simulation_execution_interface.html#section_simulation_execution_GymManager_init", null ],
          [ "コンフィグの書き換え", "page_simulation_execution_interface.html#section_simulation_execution_GymManager_reconfigure", [
            [ "インスタンス生成時の書き換え", "page_simulation_execution_interface.html#section_simulation_execution_GymManager_reconfigure_at_instantiation", null ],
            [ "エピソード開始時の書き換え", "page_simulation_execution_interface.html#section_simulation_execution_GymManager_reconfigure_on_episode_begin", null ]
          ] ]
        ] ],
        [ "SinglizedEnv クラス", "page_simulation_execution_interface.html#section_simulation_execution_SinglizedEnv", null ]
      ] ],
      [ "SimpleEvaluator", "page_simulation_execution_interface.html#section_simulation_execution_SimpleEvaluator", null ],
      [ "StandalonePolicy", "page_simulation_execution_interface.html#section_simulation_execution_StandalonePolicy", [
        [ "Agentのfull nameの書式", "page_simulation_execution_interface.html#section_simulation_execution_Agent_naming_convention", null ]
      ] ]
    ] ],
    [ "時刻系の表現について", "page_simulation_TimeSystem.html", [
      [ "Time", "page_simulation_TimeSystem.html#section_simulation_Time", [
        [ "Time クラスのjson表現", "page_simulation_TimeSystem.html#section_simulation_TimeSystem_Time_json", null ]
      ] ],
      [ "Epoch", "page_simulation_TimeSystem.html#section_simulation_Epoch", [
        [ "Epoch クラスのjson表現", "page_simulation_TimeSystem.html#section_simulation_TimeSystem_Epoch_json", null ]
      ] ]
    ] ],
    [ "座標系の取り扱いについて", "page_simulation_Coordinate.html", [
      [ "座標系の分類とクラス階層", "page_simulation_Coordinate.html#section_simulation_Coordinate_hierarchy", [
        [ "GeodeticCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_GeodeticCRS", [
          [ "制約", "page_simulation_Coordinate.html#section_simulation_Coordinate_GeodeticCRS_limitation", null ]
        ] ],
        [ "PureFlatCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_PureFlatCRS", null ],
        [ "DerivedCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_DerivedCRS", [
          [ "AffineCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_AffineCRS", null ],
          [ "TopocentricCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_TopocentricCRS", null ],
          [ "ProjectedCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_ProjectedCRS", null ]
        ] ]
      ] ],
      [ "座標軸の順序について", "page_simulation_Coordinate.html#section_simulation_Coordinate_axis_order", [
        [ "GeodeticCRS の場合", "page_simulation_Coordinate.html#section_simulation_Coordinate_axis_geodetic", null ],
        [ "物体固定座標系の直交座標軸の場合", "page_simulation_Coordinate.html#section_simulation_Coordinate_axis_body_cartesian", null ],
        [ "局所水平座標系の直交座標軸の場合", "page_simulation_Coordinate.html#section_simulation_Coordinate_axis_topocentric_cartesian", null ],
        [ "球座標系の場合", "page_simulation_Coordinate.html#section_simulation_Coordinate_axis_spherical", null ]
      ] ],
      [ "座標値の種類", "page_simulation_Coordinate.html#section_simulation_Coordinate_CoordinateType", null ],
      [ "CoordinateReferenceSystem オブジェクトを用いた座標変換", "page_simulation_Coordinate.html#section_simulation_Coordinate_CRS_transformation", null ],
      [ "座標値を表すデータ型「 Coordinate 」", "page_simulation_Coordinate.html#section_simulation_Coordinate_Coordinate", [
        [ "Coordinate クラスのjson表現", "page_simulation_Coordinate.html#section_simulation_Coordinate_Coordinate_json", null ],
        [ "Coordinate オブジェクトの座標変換", "page_simulation_Coordinate.html#section_simulation_Coordinate_Coordinate_transformation", null ]
      ] ],
      [ "CRSモデルのプリセット", "page_simulation_Coordinate.html#section_simulation_Coordinate_preset_models", [
        [ "SimulationManager固有のCRSインスタンスのプリセット", "page_simulation_Coordinate.html#section_simulation_Coordinate_preset_instances", null ]
      ] ],
      [ "シミュレーション中に必ず定義される座標系", "page_simulation_Coordinate.html#section_simulation_Coordinate_in_simulation", [
        [ "SimulationManager::rootCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_in_simulation_rootCRS", null ],
        [ "Ruler::localCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_in_simulation_Ruler_localCRS", null ],
        [ "Agent::localCRS", "page_simulation_Coordinate.html#section_simulation_Coordinate_in_simulation_Agent_localCRS", null ]
      ] ]
    ] ],
    [ "運動状態の表現方法(MotionState)", "page_simulation_MotionState.html", [
      [ "MotionState クラスのjson表現", "page_simulation_MotionState.html#section_simulation_MotionState_MotionState_json", null ],
      [ "時刻の外挿", "page_simulation_MotionState.html#section_simulation_MotionState_extrapolation", null ],
      [ "座標変換", "page_simulation_MotionState.html#section_simulation_MotionState_transformation", [
        [ "個別の状態量の座標変換", "page_simulation_MotionState.html#section_simulation_MotionState_state_transformation", null ],
        [ "MotionStateオブジェクト全体の座標変換", "page_simulation_MotionState.html#section_simulation_MotionState_entire_transformation", null ],
        [ "<a class=\"el\" href=\"page_simulation_entity_type.html#section_simulation_entity_PhysicalAsset\">PhysicalAsset</a>に固定された座標系(<a class=\"el\" href=\"page_simulation_Coordinate.html#section_simulation_Coordinate_AffineCRS\">AffineCRS</a>)としての座標変換", "page_simulation_MotionState.html#section_simulation_MotionState_transformation_as_AffineCRS", [
          [ "座標種別 xxx の選択肢", "page_simulation_MotionState.html#autotoc_md14", null ],
          [ "座標系 Y,Z の選択肢", "page_simulation_MotionState.html#autotoc_md15", null ]
        ] ]
      ] ]
    ] ],
    [ "航跡情報の表現方法(TrackBase, Track3D, Track2D)", "page_simulation_Track.html", [
      [ "3次元航跡 (Track3D)", "page_simulation_Track.html#section_simulation_Track_Track3D", [
        [ "Track3D クラスのjson表現", "page_simulation_Track.html#section_simulation_Track_Track3D_json", null ]
      ] ],
      [ "2次元航跡 (Track2D)", "page_simulation_Track.html#section_simulation_Track_Track2D", [
        [ "Track2D クラスのjson表現", "page_simulation_Track.html#section_simulation_Track_Track2D_json", null ]
      ] ],
      [ "時刻の外挿", "page_simulation_Track.html#section_simulation_Track_extrapolation", null ],
      [ "航跡のマージ", "page_simulation_Track.html#section_simulation_Track_merge", null ],
      [ "同一性の判定", "page_simulation_Track.html#section_simulation_Track_identification", null ],
      [ "座標変換", "page_simulation_Track.html#section_simulation_Track_transformation", [
        [ "個別の状態量の座標変換", "page_simulation_Track.html#section_simulation_Track_state_transformation", null ],
        [ "Trackオブジェクト全体の座標変換", "page_simulation_Track.html#section_simulation_Track_entire_transformation", null ]
      ] ]
    ] ],
    [ "CommunicationBuffer による Asset 間通信の表現", "page_simulation_CommunicationBuffer.html", null ],
    [ "Accessorクラスによるアクセス制限", "page_simulation_Accessor.html", [
      [ "SimulationManagerへのアクセス制限", "page_simulation_Accessor.html#section_simulation_accessor_SimulationManager", null ],
      [ "Entity へのアクセス制限", "page_simulation_Accessor.html#section_simulation_accessor_Entity", [
        [ "Asset へのアクセス制限", "page_simulation_Accessor.html#section_simulation_accessor_Asset", [
          [ "PhysicalAsset へのアクセス制限", "page_simulation_Accessor.html#section_simulation_accessor_PhysicalAsset", null ],
          [ "Fighter へのアクセス制限", "page_simulation_Accessor.html#section_simulation_accessor_Fighter", null ]
        ] ],
        [ "Ruler へのアクセス制限", "page_simulation_Accessor.html#section_simulation_accessor_Ruler", null ]
      ] ]
    ] ],
    [ "オブジェクトのシリアライゼーション", "page_simulation_serialization.html", [
      [ "既存オブジェクトへの参照としてのシリアライゼーション", "page_simulation_serialization.html#section_simulation_serialization_json_as_reference", [
        [ "既存の Entity インスタンスへの参照としてのjson表現", "page_simulation_serialization.html#section_simulation_Entity_as_reference_json", null ],
        [ "既存の EntityManager インスタンスへの参照としてのjson表現", "page_simulation_serialization.html#section_simulation_EntityManager_as_reference_json", null ]
      ] ],
      [ "値渡しでシリアライゼーションを行うクラス", "page_simulation_serialization.html#section_simulation_serialization_json_as_value", null ],
      [ "内部状態のシリアライゼーション (Experimental)", "page_simulation_serialization.html#autotoc_md16", [
        [ "Entity の内部状態の保存と復元", "page_simulation_serialization.html#autotoc_md17", null ],
        [ "SimulationManagerの内部状態の保存と復元", "page_simulation_serialization.html#autotoc_md18", null ]
      ] ]
    ] ],
    [ "jsonによる確率的パラメータ設定", "page_simulation_get_value_from_json.html", [
      [ "関数名に付けるK,R,Dの意味", "page_simulation_get_value_from_json.html#autotoc_md12", null ],
      [ "getValueFromJsonR(j,gen) のサンプリング仕様", "page_simulation_get_value_from_json.html#autotoc_md13", null ]
    ] ],
    [ "ConfigDispatcher によるjson object の再帰的な変換", "page_simulation_config_dispatcher.html", [
      [ "ConfigDispatcherの使用方法", "page_simulation_config_dispatcher.html#section_simulation_config_dispatcher_usage", null ],
      [ "再帰的な展開の仕様", "page_simulation_config_dispatcher.html#section_simulation_config_dispatcher_dispatch", null ]
    ] ]
];