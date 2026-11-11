<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.14.0" doxygen_gitid="cbe58f6237b2238c9af7f51c6b7afb8bbf52c866">
  <compound kind="file">
    <name>main.dox</name>
    <path>core_plugins/Supervised/doc/src/</path>
    <filename>main_8dox.html</filename>
  </compound>
  <compound kind="file">
    <name>SimpleSupervisedTaskSample.h</name>
    <path>core_plugins/Supervised/include/</path>
    <filename>SimpleSupervisedTaskSample_8h.html</filename>
    <includes id="SupervisedTaskMixer_8h" name="SupervisedTaskMixer.h" local="yes" import="no" module="no" objc="no">SupervisedTaskMixer.h</includes>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>initialize</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a93bf86397028a1470554607e94b31c08</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>serializeInternalState</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a35422d85cc3cd5e7755b5aacdaf86d84</anchor>
      <arglist>(asrc::core::util::AvailableArchiveTypes &amp;archive, bool full) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>onEpisodeEnd</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a0364534de6a5e64473fcbed5899ee9e0</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>calcTaskInputSpace</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>ada07e0cd108e9c87c21f45aba849606b</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>calcTaskTruthSpace</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>ac42245298471460d0335fb727040d74f</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>calcTaskInput</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>ae63b6fd8d02c7325e0b9d0b6f02c4eeb</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual std::pair&lt; bool, py::object &gt;</type>
      <name>calcTaskTruth</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a3a55a56f4a6ba6b5799b33a706438f03</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent) override</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportSimpleSupervisedTaskSample</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a6a1fbfa045e923b6edc851493cde0b7b</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>lower</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a3bee8d8c2ce7191603bdb42a4f1b9889</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>upper</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>a43fdb3cb04d9fec4b15d59a71a512a17</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::map&lt; std::string, int &gt;</type>
      <name>inputs</name>
      <anchorfile>SimpleSupervisedTaskSample_8h.html</anchorfile>
      <anchor>abe03a966ddcba88f98b36df2512b833e</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>SupervisedTaskMixer.h</name>
    <path>core_plugins/Supervised/include/</path>
    <filename>SupervisedTaskMixer_8h.html</filename>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>initialize</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a93bf86397028a1470554607e94b31c08</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>serializeInternalState</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a35422d85cc3cd5e7755b5aacdaf86d84</anchor>
      <arglist>(asrc::core::util::AvailableArchiveTypes &amp;archive, bool full) override</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual py::object</type>
      <name>calcTaskInputSpace</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>abcc90b737668266d30cee9b6d3d7043d</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual py::object</type>
      <name>calcTaskInput</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>abb70b528b9c94e10558438672346cf4d</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent)=0</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>overrideObservationSpaceImpl</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a7fd4328325acbcb7478fba63bf9375a0</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent, py::object oldSpace) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual py::object</type>
      <name>overrideObservationImpl</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a800126e4fd93d3aceb3dd325c1d932c5</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent, py::object oldObs) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual bool</type>
      <name>isTarget</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a39d7e705cc2d53b031a1c44355490fee</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent) override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>onEpisodeBegin</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a0b4eec1b02a02bb3c2c0d634ef8a0e8b</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="virtual">
      <type>virtual void</type>
      <name>onStepEnd</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a42be29595fde5da7f06f05d88d7fe38f</anchor>
      <arglist>() override</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual py::object</type>
      <name>calcTaskTruthSpace</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a7000d954b46da656aaabdd3e7d0a5a4a</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent)=0</arglist>
    </member>
    <member kind="function" virtualness="pure">
      <type>virtual std::pair&lt; bool, py::object &gt;</type>
      <name>calcTaskTruth</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>ac26e1b3a53d5baf1c9d7701d2c960568</anchor>
      <arglist>(const std::shared_ptr&lt; Agent &gt; &amp;agent)=0</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>exportSupervisedTaskMixer</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a19b15b505c527a56cfa31872818615d1</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
    <member kind="variable">
      <type>std::string</type>
      <name>taskName</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>afea0502cff140f1f8e683e5c443a5f2b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::map&lt; std::string, std::vector&lt; std::string &gt; &gt;</type>
      <name>targetAgentModels</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>afcc8e31408ec42713e0299e900484521</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; std::map&lt; std::string, py::object &gt; &gt;</type>
      <name>truths</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>a33cf65eefadb99458e35d0f81329558f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::map&lt; std::string, py::object &gt;</type>
      <name>truth_spaces</name>
      <anchorfile>SupervisedTaskMixer_8h.html</anchorfile>
      <anchor>aaa3b325434fb502fc9badc359ac0def7</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>Main.cpp</name>
    <path>core_plugins/Supervised/src/</path>
    <filename>Main_8cpp.html</filename>
    <member kind="function">
      <type></type>
      <name>bindSTLContainer</name>
      <anchorfile>Main_8cpp.html</anchorfile>
      <anchor>a052108ed4e2f3c0f3415cc4821f1681c</anchor>
      <arglist>(m)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>exportSupervisedTaskMixer</name>
      <anchorfile>Main_8cpp.html</anchorfile>
      <anchor>a1e6804c95e264af0790eb3edd0db1c5a</anchor>
      <arglist>(m, factoryHelper)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>exportSimpleSupervisedTaskSample</name>
      <anchorfile>Main_8cpp.html</anchorfile>
      <anchor>a02f6ace582cd9d7092cba0177fd73afd</anchor>
      <arglist>(m, factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>SimpleSupervisedTaskSample.cpp</name>
    <path>core_plugins/Supervised/src/</path>
    <filename>SimpleSupervisedTaskSample_8cpp.html</filename>
    <member kind="function">
      <type>void</type>
      <name>exportSimpleSupervisedTaskSample</name>
      <anchorfile>SimpleSupervisedTaskSample_8cpp.html</anchorfile>
      <anchor>a6a1fbfa045e923b6edc851493cde0b7b</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>SupervisedTaskMixer.cpp</name>
    <path>core_plugins/Supervised/src/</path>
    <filename>SupervisedTaskMixer_8cpp.html</filename>
    <member kind="function">
      <type>void</type>
      <name>exportSupervisedTaskMixer</name>
      <anchorfile>SupervisedTaskMixer_8cpp.html</anchorfile>
      <anchor>a19b15b505c527a56cfa31872818615d1</anchor>
      <arglist>(py::module &amp;m, const std::shared_ptr&lt; asrc::core::FactoryHelper &gt; &amp;factoryHelper)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>__init__.py</name>
    <path>core_plugins/Supervised/Supervised/</path>
    <filename>____init_____8py.html</filename>
  </compound>
  <compound kind="file">
    <name>AuxiliaryTaskUtilForHandyRL.py</name>
    <path>core_plugins/Supervised/Supervised/</path>
    <filename>AuxiliaryTaskUtilForHandyRL_8py.html</filename>
    <namespace>Supervised::AuxiliaryTaskUtilForHandyRL</namespace>
  </compound>
  <compound kind="namespace">
    <name>Supervised::AuxiliaryTaskUtilForHandyRL</name>
    <filename>namespaceSupervised_1_1AuxiliaryTaskUtilForHandyRL.html</filename>
    <member kind="function">
      <type></type>
      <name>to_torch</name>
      <anchorfile>namespaceSupervised_1_1AuxiliaryTaskUtilForHandyRL.html</anchorfile>
      <anchor>a5870da1c12de129b123fe850ed84358e</anchor>
      <arglist>(x)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>to_gpu</name>
      <anchorfile>namespaceSupervised_1_1AuxiliaryTaskUtilForHandyRL.html</anchorfile>
      <anchor>a183fe507d175130f82439ff94500f9e5</anchor>
      <arglist>(data)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>replace_none</name>
      <anchorfile>namespaceSupervised_1_1AuxiliaryTaskUtilForHandyRL.html</anchorfile>
      <anchor>a9730abcd04ed275abbd43267a5e9e887</anchor>
      <arglist>(a, b)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Supervised</title>
    <filename>index.html</filename>
    <subpage>page_MatchMaker_MatchMaker.html</subpage>
    <subpage>page_MatchMaker_TwoTeamCombatMatchMaker.html</subpage>
    <subpage>page_installation.html</subpage>
    <subpage>page_simulation.html</subpage>
    <subpage>page_training_feature.html</subpage>
    <subpage>page_simulation_basic_aac.html</subpage>
    <subpage>page_plugins.html</subpage>
    <subpage>page_r8_contest_tutorial.html</subpage>
    <docanchor file="index.html">mainpage_Supervised</docanchor>
  </compound>
</tagfile>
