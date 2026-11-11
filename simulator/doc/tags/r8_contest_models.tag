<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.14.0" doxygen_gitid="cbe58f6237b2238c9af7f51c6b7afb8bbf52c866">
  <compound kind="file">
    <name>main.dox</name>
    <path>user_plugins/r8_contest_models/doc/src/</path>
    <filename>main_8dox.html</filename>
  </compound>
  <compound kind="file">
    <name>CapsuleCoursePatrolAgent.h</name>
    <path>user_plugins/r8_contest_models/include/</path>
    <filename>CapsuleCoursePatrolAgent_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function">
      <type>void</type>
      <name>exportCapsuleCoursePatrolAgent</name>
      <anchorfile>CapsuleCoursePatrolAgent_8h.html</anchorfile>
      <anchor>a10af797b26db3311bd4f904fa97e6d6d</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Common.h</name>
    <path>user_plugins/r8_contest_models/include/</path>
    <filename>Common_8h.html</filename>
    <member kind="function">
      <type></type>
      <name>ASRC_PYBIND11_MAKE_OPAQUE</name>
      <anchorfile>Common_8h.html</anchorfile>
      <anchor>a7817461ce8fffa8268f6f5714f600f5d</anchor>
      <arglist>(std::map&lt; std::string, Eigen::Tensor&lt; double, 2 &gt; &gt;)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>PlanarFighter.h</name>
    <path>user_plugins/r8_contest_models/include/</path>
    <filename>PlanarFighter_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <includes id="R8ContestMassPointFighter_8h" name="R8ContestMassPointFighter.h" local="yes" import="no" module="no" objc="no">R8ContestMassPointFighter.h</includes>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>getDefaultCommand</name>
      <anchorfile>PlanarFighter_8h.html</anchorfile>
      <anchor>aaf95b6bd6cdc05068f8e189e329da67c</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>calc</name>
      <anchorfile>PlanarFighter_8h.html</anchorfile>
      <anchor>ae5e47ea069a0e13857616a864f7004ec</anchor>
      <arglist>(const nl::json &amp;cmd) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>calcDirect</name>
      <anchorfile>PlanarFighter_8h.html</anchorfile>
      <anchor>a5cb70db1344e271d42d6943ed96befbd</anchor>
      <arglist>(const nl::json &amp;cmd) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>calcFromDirAndVel</name>
      <anchorfile>PlanarFighter_8h.html</anchorfile>
      <anchor>a27dd8b8f83a82c7e479ed58797e4154e</anchor>
      <arglist>(const nl::json &amp;cmd) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportPlanarFighter</name>
      <anchorfile>PlanarFighter_8h.html</anchorfile>
      <anchor>a0f6c4baf836a1cb95c54e54e29676f2d</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>PlanarMissile.h</name>
    <path>user_plugins/r8_contest_models/include/</path>
    <filename>PlanarMissile_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function">
      <type>void</type>
      <name>exportPlanarMissile</name>
      <anchorfile>PlanarMissile_8h.html</anchorfile>
      <anchor>a46e74a3c428b6bcbce3a1f0ae8746e6f</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>R8ContestMassPointFighter.h</name>
    <path>user_plugins/r8_contest_models/include/</path>
    <filename>R8ContestMassPointFighter_8h.html</filename>
    <includes id="Common_8h" name="Common.h" local="yes" import="no" module="no" objc="no">Common.h</includes>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>getExtraDynamicsState</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>aeff8096751138351b98ed15b835f1506</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; double, double &gt;</type>
      <name>getVelocityRange</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a70ba660e80cb843e5677297449003421</anchor>
      <arglist>(const MotionState &amp;m, const nl::json &amp;extraArgs=nl::json::object())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; double, double &gt;</type>
      <name>getAccelRange</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a0f26272e388b135acfa43637962544ac</anchor>
      <arglist>(const MotionState &amp;m, const nl::json &amp;extraArgs=nl::json::object())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; double, double &gt;</type>
      <name>getRollRateRange</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>ade0281d100da0799bcdd7924c1ca5d20</anchor>
      <arglist>(const MotionState &amp;m, const nl::json &amp;extraArgs=nl::json::object())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; double, double &gt;</type>
      <name>getPitchRateRange</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>aca9f8a8be354ee4788dd9958b5b7e14b</anchor>
      <arglist>(const MotionState &amp;m, const nl::json &amp;extraArgs=nl::json::object())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; double, double &gt;</type>
      <name>getYawRateRange</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a572ffb251d217275333315a3df794a6c</anchor>
      <arglist>(const MotionState &amp;m, const nl::json &amp;extraArgs=nl::json::object())</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>loadTableFromJsonObject</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a94b8b5347f1cd4352a318ae5accf3199</anchor>
      <arglist>(const nl::json &amp;j_table)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>R8ContestMassPointFighterAccessor</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a27c8086bd4655cac927b2c4ef02b084d</anchor>
      <arglist>(const std::shared_ptr&lt; R8ContestMassPointFighter &gt; &amp;f)</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>calcDirect</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a5cb70db1344e271d42d6943ed96befbd</anchor>
      <arglist>(const nl::json &amp;cmd) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual nl::json</type>
      <name>calcFromDirAndVel</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a27dd8b8f83a82c7e479ed58797e4154e</anchor>
      <arglist>(const nl::json &amp;cmd) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportR8ContestMassPointFighter</name>
      <anchorfile>R8ContestMassPointFighter_8h.html</anchorfile>
      <anchor>a5883c401fbd6bf6c9c8af3c10c56daf6</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>__init__.py</name>
    <path>user_plugins/r8_contest_models/r8_contest_models/</path>
    <filename>____init_____8py.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestGodView.py</name>
    <path>user_plugins/r8_contest_models/r8_contest_models/</path>
    <filename>R8ContestGodView_8py.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestTwoTeamCombatMatchMaker.py</name>
    <path>user_plugins/r8_contest_models/r8_contest_models/</path>
    <filename>R8ContestTwoTeamCombatMatchMaker_8py.html</filename>
  </compound>
  <compound kind="file">
    <name>CapsuleCoursePatrolAgent.cpp</name>
    <path>user_plugins/r8_contest_models/src/</path>
    <filename>CapsuleCoursePatrolAgent_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>Main.cpp</name>
    <path>user_plugins/r8_contest_models/src/</path>
    <filename>Main_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>PlanarFighter.cpp</name>
    <path>user_plugins/r8_contest_models/src/</path>
    <filename>PlanarFighter_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>PlanarMissile.cpp</name>
    <path>user_plugins/r8_contest_models/src/</path>
    <filename>PlanarMissile_8cpp.html</filename>
  </compound>
  <compound kind="file">
    <name>R8ContestMassPointFighter.cpp</name>
    <path>user_plugins/r8_contest_models/src/</path>
    <filename>R8ContestMassPointFighter_8cpp.html</filename>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>r8_contest_models</title>
    <filename>index.html</filename>
    <subpage>page_MatchMaker_MatchMaker.html</subpage>
    <subpage>page_MatchMaker_TwoTeamCombatMatchMaker.html</subpage>
    <subpage>page_installation.html</subpage>
    <subpage>page_simulation.html</subpage>
    <subpage>page_training_feature.html</subpage>
    <subpage>page_simulation_basic_aac.html</subpage>
    <subpage>page_plugins.html</subpage>
    <subpage>page_r8_contest_tutorial.html</subpage>
    <docanchor file="index.html">mainpage_r8_contest_models</docanchor>
  </compound>
</tagfile>
