<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.14.0" doxygen_gitid="cbe58f6237b2238c9af7f51c6b7afb8bbf52c866">
  <compound kind="file">
    <name>main.dox</name>
    <path>sample/modules/r8_contest_sample/doc/src/</path>
    <filename>main_8dox.html</filename>
  </compound>
  <compound kind="file">
    <name>Common.h</name>
    <path>sample/modules/r8_contest_sample/include/</path>
    <filename>Common_8h.html</filename>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a4b21c8e955b3ee7489cf06dc69794d8e</anchor>
      <arglist>(std::map&lt; boost::uuids::uuid, asrc::core::Time &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a88335a29e75d8ffdd1ee2056f9b042ae</anchor>
      <arglist>(std::map&lt; std::string, Eigen::VectorX&lt; bool &gt; &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a8d94c22245c211b89859e0ceac89df08</anchor>
      <arglist>(std::vector&lt; std::weak_ptr&lt; asrc::core::Fighter &gt; &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>ad61f2e72f37dd1179683c94bfe9baa03</anchor>
      <arglist>(std::map&lt; std::string, std::vector&lt; std::weak_ptr&lt; asrc::core::Fighter &gt; &gt; &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>abc685b8bae4c12ad1df42790f18343ee</anchor>
      <arglist>(std::map&lt; std::string, std::vector&lt; std::weak_ptr&lt; asrc::core::Missile &gt; &gt; &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a4a364149656c660d3fd5cb2b314ac66b</anchor>
      <arglist>(std::unordered_map&lt; std::string, bool &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a3b62a43f46041fbb9d42000529d8bb03</anchor>
      <arglist>(std::unordered_map&lt; std::string, std::string &gt;)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>R8ContestAgentSample01.h</name>
    <path>sample/modules/r8_contest_sample/include/</path>
    <filename>R8ContestAgentSample01_8h.html</filename>
    <includes id="VirtualSimulatorSample_8h" name="VirtualSimulatorSample.h" local="yes" import="no" module="no" objc="no">VirtualSimulatorSample.h</includes>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function">
      <type></type>
      <name>ActionInfo</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a2b953e51997285a8746a7737e768d4ba</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>serialize</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a471354d71ba97866fe5dabb75f1a016b</anchor>
      <arglist>(Archive &amp;archive)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>initialize</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a93bf86397028a1470554607e94b31c08</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>serializeInternalState</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a35422d85cc3cd5e7755b5aacdaf86d84</anchor>
      <arglist>(asrc::core::util::AvailableArchiveTypes &amp;archive, bool full) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>validate</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a6029c7a91f66260ab03c9dedd7317276</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>observation_space</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ab7ffd66d94392e15484ebe38c430937f</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>makeObs</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a0a564f70dcb55d6d88eb693a02727291</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>extractFriendObservables</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>adcacebc30e896375c96bffbd22d12e25</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>extractFriendMissileObservables</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a43581928749e2393c4009db91493d7cf</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>extractEnemyObservables</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ac6606234c2ee68a5d0b4b3dbbff384c6</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>extractEnemyMissileObservables</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>afbf9fb52c1f3d74358a7b01a67130cfb</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>makeImageObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a52805bd4d3c6da64affe78177807165c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeCommonStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a3a41b4bd930f63d46ea445d632feb900</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeParentStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aef38fa58fc7df47b973499c5d75417bb</anchor>
      <arglist>(const std::shared_ptr&lt; PhysicalAssetAccessor &gt; &amp;parent, const nl::json &amp;fObs, const MotionState &amp;fMotion)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeFriendStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aa0e3b3c71d83dae384f782c0ac5abe6c</anchor>
      <arglist>(const nl::json &amp;fObs, const MotionState &amp;fMotion)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeEnemyStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a732a944763895e81348f5e8711635cca</anchor>
      <arglist>(const Track3D &amp;track)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeEnemy2DObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a0a708143d3cd6ae183c18125358a619d</anchor>
      <arglist>(const Track2D &amp;track)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeFriendMissileStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a2952aa18bf1a122589100e3460573443</anchor>
      <arglist>(const nl::json &amp;mObs)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::VectorXf</type>
      <name>makeEnemyMissileStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a0fb454a0dcae46e04431de2ebc6d04c5</anchor>
      <arglist>(const Track2D &amp;track)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::Tensor&lt; float, 1 &gt;</type>
      <name>makeFriendEnemyRelativeStateObservation</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>af8c982aa9a1ef21e554fe4f7ebe2818b</anchor>
      <arglist>(const std::shared_ptr&lt; PhysicalAssetAccessor &gt; &amp;parent, const nl::json &amp;fObs, const MotionState &amp;fMotion, const Track3D &amp;track)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>makeActionMask</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>af50ac6804e9740726ca156720d5cc4ed</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>action_space</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a9f571901519b971f1f51d766b26a9a10</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>deploy</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aadff738ed73fc5fb1a812f89279fe4c2</anchor>
      <arglist>(py::object action) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>control</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>abffae06bc588241a6923f95c1541d8ff</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>convertActionFromAnother</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a0d214eaab88ad374b592e8fb084b20bd</anchor>
      <arglist>(const nl::json &amp;decision, const nl::json &amp;command) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>controlWithAnotherAgent</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a5002eb8d28c67e63e403603126c91169</anchor>
      <arglist>(const nl::json &amp;decision, const nl::json &amp;command)</arglist>
    </member>
    <member kind="function">
      <type>bool</type>
      <name>isInside</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>afb8d597b6b791b6a5b7e6b91a38ffb40</anchor>
      <arglist>(const int &amp;lon, const int &amp;lat)</arglist>
    </member>
    <member kind="function">
      <type>Eigen::Vector2i</type>
      <name>rposToGrid</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ac9f0fd22d38fbd41e7c1bc189a328235</anchor>
      <arglist>(const Eigen::Vector3d &amp;dr)</arglist>
    </member>
    <member kind="function">
      <type>Eigen::Vector3d</type>
      <name>gridToRpos</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ae893241de0b9354982a1ba154ab49e82</anchor>
      <arglist>(const int &amp;lon, const int &amp;lat, const double &amp;alt)</arglist>
    </member>
    <member kind="function">
      <type>Eigen::Vector2i</type>
      <name>posToGrid</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a6e0fc6a2d4556f09b47372d0959bd8c8</anchor>
      <arglist>(const Eigen::Vector3d &amp;pos)</arglist>
    </member>
    <member kind="function">
      <type>Eigen::Vector3d</type>
      <name>gridToPos</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a1baa3ad4d44f155c756b97451c3650f4</anchor>
      <arglist>(const int &amp;lon, const int &amp;lat, const double &amp;alt)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>plotPoint</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a40032eeb404605b9c948930c79ddfec2</anchor>
      <arglist>(const Eigen::Vector3d &amp;pos, const int &amp;ch, const float &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>plotLine</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ad6e5c4d2c151c2e5d876c4dd8639d98f</anchor>
      <arglist>(const Eigen::Vector3d &amp;pBegin, const Eigen::Vector3d &amp;pEnd, const int &amp;ch, const float &amp;value)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportR8ContestAgentSample01</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a8b885e0e56b795b2797eaa06580c00d9</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
    <member kind="function">
      <type>ASRC_PLUGIN_NAMESPACE_END</type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a5abf6a55c8e229a06017fa12fa2f0d18</anchor>
      <arglist>(std::vector&lt; r8_contest_sample::R8ContestAgentSample01::InstantInfo &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a19dc85115b113e9d33f257c884ccb778</anchor>
      <arglist>(std::vector&lt; r8_contest_sample::R8ContestAgentSample01::ActionInfo &gt;)</arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; InstantInfo &gt;</type>
      <name>image_past_data</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a85a31894681269ac220627d98d4e8f0c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>common_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a5af088e842ed3112bd3862a64ba6ed8d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>parent_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a220e40e52d5be53136589a3436f9c592</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>friend_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a076a22da44e51c29d03146b1b3d631aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>enemy_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a88ab141b13bbf0f92ed36451b721fef4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>enemy_2d_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a7532bd202c6b1659ea285feac57044ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>friend_missile_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>acf0116a03010b2495e551659399f27b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>enemy_missile_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a44448218a23e5e269c7fc08b9c7ece05</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>friend_enemy_relative_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a13c7bbfa5436ef61b3c3aada69023702</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>last_action_dim</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ad9696a83b127cb4e26089076f776eb66</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::map&lt; std::string, Eigen::VectorXf &gt;</type>
      <name>last_action_obs</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a5ed007d60aef4e69dddf4bee1eae4cb9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>py::object</type>
      <name>_action_space</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a832cc923a9091c30058d014c277f1ba0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Eigen::Vector3d</type>
      <name>dstDir</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a1fd61ca784ea1cf7932738b19e81570a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dstAlt</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aef65e6155c871b3bd9cf2a09298ec910</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>velRecovery</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ae468e4164c9e417c582eb982560bd5ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>asThrottle</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a8a3b4e94a70d7bbae433c76caede88db</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>keepVel</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ae2b54c6d5a1f09d8ba66be837d30fd0b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dstThrottle</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ac881271affeb5659826d449e454c4db5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dstV</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aab2ce3400f0157a09b5ac3de0130e3bc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>launchFlag</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a5169338454508001069c1281ae55ed9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Track3D</type>
      <name>target</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a2e3bc3ee7ab0959dd4211bc19854a267</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::map&lt; boost::uuids::uuid, Time &gt;</type>
      <name>lastShotTimes</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aa531ae247ebdd0a73a10402b51e3dab7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::map&lt; std::string, ActionInfo &gt;</type>
      <name>actionInfos</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>ae4730526f81bb7d9ee7ffca25ea7f4cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>py::object</type>
      <name>action_mask</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>af0572f5de01e1f56a157dd9bcfa167de</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOut</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>aeb0215911f488f62c1c9c348f64fe46d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dLine</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>a327c2569eaeb909e8ba901da0e4027a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>BasicAgentUtility::util::TeamOrigin</type>
      <name>teamOrigin</name>
      <anchorfile>R8ContestAgentSample01_8h.html</anchorfile>
      <anchor>add1bbf13f26503d589b50865752eb5ca</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>R8ContestRewardSample01.h</name>
    <path>sample/modules/r8_contest_sample/include/</path>
    <filename>R8ContestRewardSample01_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function">
      <type>void</type>
      <name>exportR8ContestRewardSample01</name>
      <anchorfile>R8ContestRewardSample01_8h.html</anchorfile>
      <anchor>a45ebc356c3a07407955cd7883412a8f8</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>R8ContestRewardSample02.h</name>
    <path>sample/modules/r8_contest_sample/include/</path>
    <filename>R8ContestRewardSample02_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function">
      <type>void</type>
      <name>exportR8ContestRewardSample02</name>
      <anchorfile>R8ContestRewardSample02_8h.html</anchorfile>
      <anchor>adcabb9b0a2f08ad64910dba91f1b41e9</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>VirtualSimulatorSample.h</name>
    <path>sample/modules/r8_contest_sample/include/</path>
    <filename>VirtualSimulatorSample_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>launchCondition</name>
      <anchorfile>VirtualSimulatorSample_8h.html</anchorfile>
      <anchor>a7c1987928c5f0f7688b703bb8b6c44d8</anchor>
      <arglist>(const std::shared_ptr&lt; asrc::core::PhysicalAssetAccessor &gt; &amp;parent, const asrc::core::MotionState &amp;motion, const asrc::core::Track3D &amp;track) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportVirtualSimulatorSample</name>
      <anchorfile>VirtualSimulatorSample_8h.html</anchorfile>
      <anchor>ae393ce6e021448e89b4aeaa9854ebcaf</anchor>
      <arglist>(py::module &amp;m)</arglist>
    </member>
    <member kind="function">
      <type>ASRC_PLUGIN_NAMESPACE_END</type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>VirtualSimulatorSample_8h.html</anchorfile>
      <anchor>a07e70c12fa4c933d2d7eaea3b57d3724</anchor>
      <arglist>(std::map&lt; std::string, std::vector&lt; std::shared_ptr&lt; asrc::core::Missile &gt; &gt; &gt;)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>__init__.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>____init_____8py.html</filename>
  </compound>
  <compound kind="file">
    <name>GenericTorchModelUtil.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>GenericTorchModelUtil_8py.html</filename>
    <namespace>r8_contest_sample::GenericTorchModelUtil</namespace>
  </compound>
  <compound kind="file">
    <name>PyVirtualSimulatorSample.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>PyVirtualSimulatorSample_8py.html</filename>
    <namespace>r8_contest_sample::PyVirtualSimulatorSample</namespace>
  </compound>
  <compound kind="file">
    <name>R8ContestPyAgentSample01.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>R8ContestPyAgentSample01_8py.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestPyRewardSample01.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>R8ContestPyRewardSample01_8py.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestPyRewardSample02.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>R8ContestPyRewardSample02_8py.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestTorchNNSampleForHandyRL.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>R8ContestTorchNNSampleForHandyRL_8py.html</filename>
    <namespace>r8_contest_sample::R8ContestTorchNNSampleForHandyRL</namespace>
  </compound>
  <compound kind="file">
    <name>R8ContestTorchNNSampleForRay.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>R8ContestTorchNNSampleForRay_8py.html</filename>
    <namespace>r8_contest_sample::R8ContestTorchNNSampleForRay</namespace>
  </compound>
  <compound kind="file">
    <name>SetTransformer.py</name>
    <path>sample/modules/r8_contest_sample/r8_contest_sample/</path>
    <filename>SetTransformer_8py.html</filename>
    <namespace>r8_contest_sample::SetTransformer</namespace>
  </compound>
  <compound kind="file">
    <name>Main.cpp</name>
    <path>sample/modules/r8_contest_sample/src/</path>
    <filename>Main_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestAgentSample01.cpp</name>
    <path>sample/modules/r8_contest_sample/src/</path>
    <filename>R8ContestAgentSample01_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestRewardSample01.cpp</name>
    <path>sample/modules/r8_contest_sample/src/</path>
    <filename>R8ContestRewardSample01_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestRewardSample02.cpp</name>
    <path>sample/modules/r8_contest_sample/src/</path>
    <filename>R8ContestRewardSample02_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>VirtualSimulatorSample.cpp</name>
    <path>sample/modules/r8_contest_sample/src/</path>
    <filename>VirtualSimulatorSample_8cpp.html</filename>
  </compound>
  <compound kind="namespace">
    <name>r8_contest_sample::GenericTorchModelUtil</name>
    <filename>namespacer8__contest__sample_1_1GenericTorchModelUtil.html</filename>
  </compound>
  <compound kind="namespace">
    <name>r8_contest_sample::PyVirtualSimulatorSample</name>
    <filename>namespacer8__contest__sample_1_1PyVirtualSimulatorSample.html</filename>
  </compound>
  <compound kind="namespace">
    <name>r8_contest_sample::R8ContestTorchNNSampleForHandyRL</name>
    <filename>namespacer8__contest__sample_1_1R8ContestTorchNNSampleForHandyRL.html</filename>
  </compound>
  <compound kind="namespace">
    <name>r8_contest_sample::R8ContestTorchNNSampleForRay</name>
    <filename>namespacer8__contest__sample_1_1R8ContestTorchNNSampleForRay.html</filename>
    <member kind="variable">
      <type></type>
      <name>torch</name>
      <anchorfile>namespacer8__contest__sample_1_1R8ContestTorchNNSampleForRay.html</anchorfile>
      <anchor>a9488da2663aea034394738a95e361786</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type></type>
      <name>nn</name>
      <anchorfile>namespacer8__contest__sample_1_1R8ContestTorchNNSampleForRay.html</anchorfile>
      <anchor>a080703b5891754c7a8856140995a0e24</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type></type>
      <name>logger</name>
      <anchorfile>namespacer8__contest__sample_1_1R8ContestTorchNNSampleForRay.html</anchorfile>
      <anchor>ad5bc31b6445c702ca7231b4dde637e72</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>r8_contest_sample::SetTransformer</name>
    <filename>namespacer8__contest__sample_1_1SetTransformer.html</filename>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>r8_contest_sample</title>
    <filename>index.html</filename>
    <subpage>page_MatchMaker_MatchMaker.html</subpage>
    <subpage>page_MatchMaker_TwoTeamCombatMatchMaker.html</subpage>
    <subpage>page_installation.html</subpage>
    <subpage>page_simulation.html</subpage>
    <subpage>page_training_feature.html</subpage>
    <subpage>page_simulation_basic_aac.html</subpage>
    <subpage>page_plugins.html</subpage>
    <subpage>page_r8_contest_tutorial.html</subpage>
    <docanchor file="index.html">mainpage_r8_contest_sample</docanchor>
  </compound>
</tagfile>
