<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.14.0" doxygen_gitid="cbe58f6237b2238c9af7f51c6b7afb8bbf52c866">
  <compound kind="file">
    <name>__init__.py</name>
    <path>user_plugins/BasicAirToAirCombatModels01/BasicAirToAirCombatModels01/</path>
    <filename>____init_____8py.html</filename>
  </compound>
  <compound kind="file">
    <name>main.dox</name>
    <path>user_plugins/BasicAirToAirCombatModels01/doc/src/</path>
    <filename>main_8dox.html</filename>
  </compound>
  <compound kind="file">
    <name>BasicAACReward01.h</name>
    <path>user_plugins/BasicAirToAirCombatModels01/include/</path>
    <filename>BasicAACReward01_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <includes id="BasicAACRuler01_8h" name="BasicAACRuler01.h" local="yes" import="no" module="no" objc="no">BasicAACRuler01.h</includes>
    <member kind="function" virtualness="virtual">
      <type>virtual double</type>
      <name>getRHit</name>
      <anchorfile>BasicAACReward01_8h.html</anchorfile>
      <anchor>afa329eae39ae73c3fe49c8b62ebff053</anchor>
      <arglist>(const std::string &amp;team, const std::string &amp;modelName) const override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual double</type>
      <name>getRHitE</name>
      <anchorfile>BasicAACReward01_8h.html</anchorfile>
      <anchor>a971307e4ad230b4d8ca9e7db2c34d9ab</anchor>
      <arglist>(const std::string &amp;team, const std::string &amp;modelName) const override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual double</type>
      <name>getRCrash</name>
      <anchorfile>BasicAACReward01_8h.html</anchorfile>
      <anchor>ac88f96c984ae15d395b2401984e001fb</anchor>
      <arglist>(const std::string &amp;team, const std::string &amp;modelName) const override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual double</type>
      <name>getRCrashE</name>
      <anchorfile>BasicAACReward01_8h.html</anchorfile>
      <anchor>a05a68e3868a302f69dbaa5acbb3f0f52</anchor>
      <arglist>(const std::string &amp;team, const std::string &amp;modelName) const override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportBasicAACReward01</name>
      <anchorfile>BasicAACReward01_8h.html</anchorfile>
      <anchor>a5535236adbb86a103e30cd8c2e135773</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>BasicAACRuleBasedAgent01.h</name>
    <path>user_plugins/BasicAirToAirCombatModels01/include/</path>
    <filename>BasicAACRuleBasedAgent01_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="enumeration">
      <type></type>
      <name>TransitionCause</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a53013cc1112b705b50caafb3fa36199f</anchor>
      <arglist></arglist>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199face8531c749e7aa12367fcf836ff4542c">FUEL_SHORTAGE</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fa943adaff834749fb93af2458db027b20">NO_ENEMY</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fad6b0e6b5079be63dbf57ebb87783b37d">TRY_BREAK</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fa7794175cb243e97a84e08b4794fd3d1f">INTERCEPT_LEAKER</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fab484e4a170468f95f5ae0e1287546a51">TRACKING_BY_ALLY</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fa0913eb30e537f07a672819cf0fdf2ec2">TRACKING_BY_NOBODY</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fad326cca73ff1664732f5da9f90ca319c">TRACKING_BY_MYSELF</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fa88ec4386a3c49b50819358a579fb9adb">ABNORMAL</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a53013cc1112b705b50caafb3fa36199fab50339a10e1de285ac99d4c3990b8693">NONE</enumvalue>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>State</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a5d74787dedbc4e11c1ab15bf487e61f8</anchor>
      <arglist></arglist>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8acc52554c5f7b607e139b80f23b8cf5d0">ADVANCE</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8a3427cb6bd8c5e0e351f6c2bea9768e42">APPROACH_TARGET</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8aa655b071c8c5190ea023aaf1752db191">KEEP_SENSING</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8aabfc5761f2bc4dde90afad63cc13f324">RTB</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8ad46cbcb197473899801adc52f1d94deb">WITHDRAW</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8a5f7b0f8d876e1a79eac3c2d62c6b3585">EVADE</enumvalue>
      <enumvalue file="BasicAACRuleBasedAgent01_8h.html" anchor="a5d74787dedbc4e11c1ab15bf487e61f8ab50339a10e1de285ac99d4c3990b8693">NONE</enumvalue>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>update</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a78955267b9a2e7e71098f5fc56f6745d</anchor>
      <arglist>(const TrackInfo &amp;other, bool isFull) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>initialize</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a93bf86397028a1470554607e94b31c08</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>serializeInternalState</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a35422d85cc3cd5e7755b5aacdaf86d84</anchor>
      <arglist>(asrc::core::util::AvailableArchiveTypes &amp;archive, bool full) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>validate</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a6029c7a91f66260ab03c9dedd7317276</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>deploy</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aadff738ed73fc5fb1a812f89279fe4c2</anchor>
      <arglist>(py::object action) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>perceive</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a12959000b8400811be4d03adc4e4d551</anchor>
      <arglist>(bool inReset) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>control</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>abffae06bc588241a6923f95c1541d8ff</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>updateMyInfo</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ab584097c2ba32a33cf25e0ffb9c961e3</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>updateTracks</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a2a6db190c17262daf34d81ae4eeba114</anchor>
      <arglist>(bool isFull)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>updateTrackInfo</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a4130d2fc493a1db8d31e9a19a080bb3b</anchor>
      <arglist>(TrackInfo &amp;ti, bool isFull)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>updateTargetStatus</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a614dee105f013a1584ce3bc0bfe2c9b7</anchor>
      <arglist>(bool isFull)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>updateTargetStatusSub</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>abb36d32d02e12a3a91fdacfe6e74fd22</anchor>
      <arglist>(TrackInfo &amp;tgt, bool isFull)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>updateMWS</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a18097a87adc951d838cfd53c2a86ce5e</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>chkFuelSufficiency</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a85a405270dde153df724bd8b17648ed5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>chkLaunchable</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aeb7624b8dbede72ee467eb438920e2ff</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; bool, std::pair&lt; TrackInfo, double &gt; &gt;</type>
      <name>chkConditionForShoot</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a691aba28a1f7b5083adb31fdf9d03883</anchor>
      <arglist>(TrackInfo &amp;tgt)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::vector&lt; bool &gt;</type>
      <name>chkBreak</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a74148fc8e4d3792c219fa9536692af69</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>chkTransitionCount</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a3450da769125815a891e8ee38b4b85a5</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>reserveTransition</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ad62caf03ae1493fc3219941444743b53</anchor>
      <arglist>(const State &amp;dst, double delay)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>completeTransition</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a8e13f1684264179aa3df939098002e19</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>cancelTransition</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aca31a63e1c52842bef688ee9932cd7d2</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>immidiateTransition</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ae348dfe3106d96941257087d713acca1</anchor>
      <arglist>(const State &amp;dst)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>selectTarget</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aa3d5360d788b13fccaa688afe7c21ea7</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>decideShoot</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ac26baf71fee8459bcb485ee76bb727be</anchor>
      <arglist>(TrackInfo &amp;tgt)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>chooseTransitionNormal</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ac35f9e89ef699e5afbfc63e275bbe5e0</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>deploySub</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>add0532f2c9c57cdca628ffe8acaab756</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::Vector3d</type>
      <name>steerForSensingSubTargets</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a138665bf466939d78f4a1e5f207cad8a</anchor>
      <arglist>(const MotionState &amp;myMotion, const Eigen::Vector3d &amp;dstDirH)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::Vector3d</type>
      <name>keepInCombatArea</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a36f240894ee66300ce7413addb9b66e6</anchor>
      <arglist>(const MotionState &amp;myMotion, const Eigen::Vector3d &amp;dstDirH)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual Eigen::Vector3d</type>
      <name>calcVerticalManeuver</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a27e8d543836f37f9ad8ba3e9fb3ce794</anchor>
      <arglist>(const MotionState &amp;myMotion, const Eigen::Vector3d &amp;dstDirH)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>makeCommand</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a033f16d30d1d7494828a33750fdfa64e</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual double</type>
      <name>calcRHead</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ab1dabb72f679d22357a06f33444681ca</anchor>
      <arglist>(const Eigen::Vector3d &amp;rs, const Eigen::Vector3d &amp;vs, const Eigen::Vector3d &amp;rt, const Eigen::Vector3d &amp;vt)</arglist>
    </member>
    <member kind="function" protection="protected" virtualness="virtual">
      <type>virtual double</type>
      <name>calcRTail</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a8fc8eded789ac4514675f3570c2c87f3</anchor>
      <arglist>(const Eigen::Vector3d &amp;rs, const Eigen::Vector3d &amp;vs, const Eigen::Vector3d &amp;rt, const Eigen::Vector3d &amp;vt)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>void</type>
      <name>exportBasicAACRuleBasedAgent01</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a0a4307c953fad8d11c39f1155a1e940a</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type>ASRC_PLUGIN_NAMESPACE_END</type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a75a6a01b25f1b7152a4bf958ba897ea8</anchor>
      <arglist>(std::vector&lt; BasicAirToAirCombatModels01::TrackInfo &gt;)</arglist>
    </member>
    <member kind="function" protection="protected">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a4549de86950386448884abdc5395abd2</anchor>
      <arglist>(std::vector&lt; std::vector&lt; BasicAirToAirCombatModels01::TrackInfo &gt; &gt;)</arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>tMaxMemory</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a868b3b81d2cbf373b7cf4f914ec2d9a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>sensorInRangeLimit</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ae26ac7b4351636f4e75b54acf3d5632c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>sensorInCoverageLimit</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a32866a92534af7946deeca3dfcd6cbe1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>considerStateForTrackingCondition</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a672fe2e4f77f737a40728201188d6f38</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>kShoot</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aa4c3c9642fa8f947ec4419a0b888dfbb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>nMslSimul</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a06b444b636fd171cd2ff264f0096c70b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>kBreak</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ad381494e693e274ffd3b37ef7344b95e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>tWithdraw</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>af0f9499eb9418970ba49436aac93908a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>pAdvanceAlly</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a827270c808407f8f2f574aa1d3d3a97a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>pApproachAlly</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a968facc255bfa6c1a08d6d2740b20260</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>pKeepSensingAlly</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a93a59aa9d9813a836e982eb2f07194c2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>pApproachMyself</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aab948580e322f26e72df65befd75b5ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>pKeepSensingMyself</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aaf44e3f2a9aa8f14f586f14457d400af</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>reconsiderationCycle</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a305b03589d79f799380bda72a3e270f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dPrioritizedAdvance</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a7d1434908d12753c761ddeb98bfd88cd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaPrioritizedAdvance</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ae7c072088aa380f4dc9b8970d6b8436a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaKeepSensing</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a4bcc73a6fce63298a2e4dda5d210bef0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>RTBEnterMargin</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a3a23259a0b1436fe5510f9e0d56401c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>RTBExitMargin</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ada9a5b354ac6ab4463426ffc041193dd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>RTBAltitude</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a3b25eb9b2cad849b8d9b00c258e1bb0c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>RTBVelocity</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a554aa19a27ee22b5bc691f8c76f22ed2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>RTBPitchLimit</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a85d2d3e55c6ab993219200030927898c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaEvasion</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a0991c193300647127b6f371a911536f5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>hEvasion</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a83da6ca11da1220f8ada852d75111e9c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaModForSensing</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a305cfbdb65938520251a9ea784ba57a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaNormal</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a0c5db9815b8ee3547e5def0ae77a7ffc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>hNormal</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a177678d292abb99d8175d126f39efd6f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaWithdraw</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a795f67d5cd76a22800c393c41bb3ff02</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>hWithdraw</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aa4b0b3399e355e3ed9650dedf2354d5d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOutLimit</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a2a4c851e0311d00c9787160fb0c3881c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOutLimitTurnAxis</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aa48e7af44ad337c363104c306040180a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOutLimitKeepSensing</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a36c1ce4cb1335798fbbadd41b0504cf9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOutLimitThreshold</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a6ec0cf0a7852a784ad41740dddfdc037</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOutLimitStrength</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a430afc7ad560281568a4a05838ce1ff8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>regardDefenseLineAsPenaltyLine</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a0b3443f2a34a6e74afee4280eb41db93</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dPrioritizedAimLeaker</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ac3e3216fd40eba0ff480f67d606212ee</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>thetaPrioritizedAimLeaker</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a4fb13f91098a13a305f917e21cd5d2b4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>minimumV</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a5768bb8cc65672d60bb410cce0a30f80</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>minimumRecoveryV</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>acc829817212ebee5c66e195917c045f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>minimumRecoveryDstV</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a85e6ebf605e683a40ac1441e5ac193d8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>nominalThrottle</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a68e8aa3fb3f2cfbd38c0c1710a2ffaab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>cmdDeltaAzLimit</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a74a5a18ae0cfb9454f3e854cbad156d4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>State</type>
      <name>state</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a876b486d3a5241a126bd5751c5f70f79</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>MotionState</type>
      <name>myMotion</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a8134e43b2d7fbb81a977b700d38c7b2c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; TrackInfo &gt;</type>
      <name>track</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a2d0c72abb916d3a90d8c0ad6bc202b9f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; TrackInfo &gt;</type>
      <name>additionalTargets</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a6c4776155843dd82a21bf3b3a2d36de5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; TrackInfo &gt;</type>
      <name>allEnemies</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a598c166677a271d9e7892bf79f90373f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; TrackInfo &gt;</type>
      <name>withdrawnTrigger</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a439c6e685626cce292383c98726ff330</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; Track2D &gt;</type>
      <name>mwsTracks</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a323c1645178c870c73c075593230950a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>TrackInfo</type>
      <name>target</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ad7787eff70f75979f7fe5027edd28029</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>isAssignedAsInterceptor</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a894d90838334c27e6ca5b7a658153aed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>launchFlag</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a5169338454508001069c1281ae55ed9a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>velRecovery</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ae468e4164c9e417c582eb982560bd5ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>TrackInfo</type>
      <name>launchedTarget</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a2f30ed3aa5007d2e8fef32aa1304c67c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>TrackInfo</type>
      <name>fireTgt</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ad578bede08ee507454fec3527275b8ab</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Time</type>
      <name>withdrawnTime</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a073752747c177ba1bbdb2ac086fe678f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>waitingTransition</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a8f9b453f066e9bee4830cf846675e770</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>transitionDelay</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a003ca2d8fb340fc9b35127d24a20f336</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Time</type>
      <name>transitionTriggeredTime</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a844a4846d0871e636ea5b1793f7523b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>State</type>
      <name>transitionBefore</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a6bdb7063ff6fa66aa7fd671733ca4c64</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>State</type>
      <name>transitionAfter</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aaf69eded72306c0edc58e44f571514e0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>TransitionCause</type>
      <name>transitionCause</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ab093a9ad4fc913523fccb7e08e236c16</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Time</type>
      <name>lastNormalTransitionTime</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a922af64616fda13c411bf6d1dd7fd567</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>isAfterDeploy</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a63f8ea9ef5d1bbddea6fc7e0e0c63148</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Eigen::Vector3d</type>
      <name>forward</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a880576528e167caf8c0abcc05707591d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Eigen::Vector3d</type>
      <name>rightSide</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>adda6c54c7b2a37ac4e8147bd79e7e47d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dOut</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aeb0215911f488f62c1c9c348f64fe46d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>dLine</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a327c2569eaeb909e8ba901da0e4027a4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Eigen::Vector3d</type>
      <name>pos</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>ab78a91752eba49af56e4358e5f2ffe08</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Eigen::Vector3d</type>
      <name>vel</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>acb6846c6fbb7bdc86c4561f3c34ef0c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Eigen::Vector3d</type>
      <name>dstDir</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>a1fd61ca784ea1cf7932738b19e81570a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>AltitudeKeeper</type>
      <name>altitudeKeeper</name>
      <anchorfile>BasicAACRuleBasedAgent01_8h.html</anchorfile>
      <anchor>aea8def914777e595735deb4f00860267</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>BasicAACRuler01.h</name>
    <path>user_plugins/BasicAirToAirCombatModels01/include/</path>
    <filename>BasicAACRuler01_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function" virtualness="virtual">
      <type>virtual std::string</type>
      <name>getEndReasonSub</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>a2833210053897ef5062e60516152fa88</anchor>
      <arglist>() const override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>setEndReasonSub</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>a958fd14729258e471c0b3cdef0618507</anchor>
      <arglist>(const std::string &amp;newReason) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>onCrash</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>ad302cfd90668514b6df93b4ad375a2d2</anchor>
      <arglist>(const nl::json &amp;args) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>onHit</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>a436cd56e557f7a471e3133b9127d06de</anchor>
      <arglist>(const nl::json &amp;args) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>_setupPDownConfig</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>ad18e4d4a29ec4fb122326f27988f2386</anchor>
      <arglist>(std::map&lt; std::string, double &gt; &amp;_config, const nl::json &amp;_modelConfig, const std::string &amp;_key, double _defaultValue) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>_setupPDownScale</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>ae75bd9caba0788d2ba88da340e69dede</anchor>
      <arglist>(std::map&lt; std::string, std::map&lt; std::string, double &gt; &gt; &amp;_scale, const std::map&lt; std::string, double &gt; &amp;_config, bool _perAircraft) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual double</type>
      <name>_getPDownImpl</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>ad629e5794aeef86f43514730d9211319</anchor>
      <arglist>(const std::map&lt; std::string, std::map&lt; std::string, double &gt; &gt; &amp;_scale, const std::map&lt; std::string, double &gt; &amp;_config, const std::string &amp;_team, const std::string &amp;_modelName) const override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportBasicAACRuler01</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>a948ad79f3d3cd4ff9c81aafec6628470</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
    <member kind="function">
      <type>ASRC_PLUGIN_NAMESPACE_END</type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>BasicAACRuler01_8h.html</anchorfile>
      <anchor>add870564b4348917dde53e5a18b39396</anchor>
      <arglist>(std::map&lt; std::string, BasicAirToAirCombatModels01::BasicAACRuler01::DownReason &gt;)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Common.h</name>
    <path>user_plugins/BasicAirToAirCombatModels01/include/</path>
    <filename>Common_8h.html</filename>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a92a16b6ea4a7a6f5ee534c382294366f</anchor>
      <arglist>(std::map&lt; std::string, std::vector&lt; std::string &gt; &gt;)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a7901da6b4d8240cea9187a16f7b50842</anchor>
      <arglist>(std::map&lt; std::string, std::map&lt; std::string, int &gt; &gt;)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>BasicAACReward01.cpp</name>
    <path>user_plugins/BasicAirToAirCombatModels01/src/</path>
    <filename>BasicAACReward01_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>BasicAACRuleBasedAgent01.cpp</name>
    <path>user_plugins/BasicAirToAirCombatModels01/src/</path>
    <filename>BasicAACRuleBasedAgent01_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>BasicAACRuler01.cpp</name>
    <path>user_plugins/BasicAirToAirCombatModels01/src/</path>
    <filename>BasicAACRuler01_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>Main.cpp</name>
    <path>user_plugins/BasicAirToAirCombatModels01/src/</path>
    <filename>Main_8cpp.html</filename>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>BasicAirToAirCombatModels01</title>
    <filename>index.html</filename>
    <subpage>page_MatchMaker_MatchMaker.html</subpage>
    <subpage>page_MatchMaker_TwoTeamCombatMatchMaker.html</subpage>
    <subpage>page_installation.html</subpage>
    <subpage>page_simulation.html</subpage>
    <subpage>page_training_feature.html</subpage>
    <subpage>page_simulation_basic_aac.html</subpage>
    <subpage>page_plugins.html</subpage>
    <subpage>page_r8_contest_tutorial.html</subpage>
    <docanchor file="index.html">mainpage_BasicAirToAirCombatModels01</docanchor>
  </compound>
</tagfile>
