var page_simulation_entity =
[
    [ "シミュレーション登場物の種類について", "page_simulation_entity_type.html", [
      [ "クラス、モデル、ポリシーの考え方", "page_simulation_entity_type.html#section_class_model_policy", null ],
      [ "登場物の基底クラス「Entity」", "page_simulation_entity_type.html#section_simulation_entity_Entity", [
        [ "Asset", "page_simulation_entity_type.html#section_simulation_entity_Asset", [
          [ "PhysicalAsset", "page_simulation_entity_type.html#section_simulation_entity_PhysicalAsset", null ],
          [ "Controller", "page_simulation_entity_type.html#section_simulation_entity_Controller", null ],
          [ "Agent", "page_simulation_entity_type.html#section_simulation_entity_Agent", null ]
        ] ],
        [ "Callback", "page_simulation_entity_type.html#section_simulation_entity_Callback", [
          [ "Ruler", "page_simulation_entity_type.html#section_simulation_entity_Ruler", null ],
          [ "Reward", "page_simulation_entity_type.html#section_simulation_entity_Reward", null ],
          [ "Viewer", "page_simulation_entity_type.html#section_simulation_entity_Viewer", null ],
          [ "その他のCallback", "page_simulation_entity_type.html#section_simulation_entity_OtherCallback", null ]
        ] ]
      ] ]
    ] ],
    [ "Entity の基本仕様", "page_simulation_entity_basic_info.html", [
      [ "Entity 及び派生クラスの共通コンストラクタ", "page_simulation_entity_basic_info.html#section_simulation_entity_creation_Entity_constructor", null ],
      [ "Entity インスタンスの初期化処理", "page_simulation_entity_basic_info.html#section_simulation_entity_creation_initialization", null ],
      [ "処理周期の指定", "page_simulation_entity_basic_info.html#section_simulation_entity_period", [
        [ "Asset の処理周期", "page_simulation_entity_basic_info.html#section_simulation_entity_period_Asset", null ],
        [ "Callback の処理周期", "page_simulation_entity_basic_info.html#section_simulation_entity_period_Callback", null ]
      ] ],
      [ "処理順序の指定", "page_simulation_entity_basic_info.html#section_simulation_entity_dependency", null ]
    ] ],
    [ "Factory への Entity クラス・モデルの登録", "page_simulation_entity_Factory_registration.html", [
      [ "クラスの登録", "page_simulation_entity_Factory_registration.html#section_Factory_class_registration", null ],
      [ "モデルの登録", "page_simulation_entity_Factory_registration.html#section_Factory_model_registration", null ],
      [ "Factoryによるクラス・モデルの検索", "page_simulation_entity_Factory_registration.html#section_simulation_Factory_resolveModelConfig", null ]
    ] ],
    [ "Entity の生成", "page_simulation_entity_creation.html", [
      [ "EntityManager (SimulationManager)のメンバによる生成", "page_simulation_entity_creation.html#section_simulation_entity_creation_by_Manager", null ],
      [ "Entityのメンバによる生成", "page_simulation_entity_creation.html#section_simulation_entity_creation_by_Entity", null ],
      [ "Entity 生成用関数に与えるコンフィグの書式", "page_simulation_entity_creation.html#section_simulation_entity_creation_format", null ]
    ] ]
];