# Copyright (c) 2024-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
# setuptoolsのegg_info, sdist, build, install (, clean)クラス
#
# [tool.ASRCAISim1]
# is-core-plugin = false
# enable-build-identification = true
# core-plugins = [
#    # (Optional) Enumerate build-time required core plugins.
#    # If omitted, all of the installed plugins based on "requires" in [build-system] table
#    # will be regarded as dependency.
#    "CorePlugin1",
#    "CorePlugin2",
# ]
# user-plugins = [
#    # (Optional) Enumerate build-time required user plugins.
#    # If omitted, all of the installed plugins based on "requires" in [build-system] table
#    # will be regarded as dependency.
#    "UserPlugin1",
# ]
# source-encoding = "utf-8" # (Optional) Specify if you wish to use encoding other than utf-8 (e.g. cp932(Shift-JIS) in Windows environment.)
#

import sys
import os
import shutil
import datetime
import re
import glob
from setuptools.command.egg_info import egg_info as egg_info_orig
from setuptools.command.sdist import sdist as sdist_orig
from setuptools.command.build import build as build_orig
from setuptools.command.install import install as install_orig
from distutils.command.clean import clean as clean_orig
from setuptools import find_packages
from setuptools.errors import (
    OptionError,
    SetupError,
)

def get_plugin_directories(req):
    ret = []
    for p in req:
        import importlib
        try:
            m = importlib.import_module(p)
            ret.append(os.path.abspath(os.path.dirname(m.__file__)).replace(os.path.sep,'/'))
        except ImportError as e:
            print(f"Required plugin {p} could not be imported!")
            raise e
    return ";".join(ret)

def extract_package_name(dep_str: str) -> str:
    pattern = re.compile(r"[A-Z0-9][A-Z0-9._-]*[A-Z0-9]|[A-Z0-9]", re.IGNORECASE)
    res = pattern.search(dep_str)
    if res:
        return res[0]
    else:
        raise ValueError(f"'{dep_str}' does not contain a valid package name")

def get_depending_plugins(tomlfilepath: str) -> set:
    if sys.version_info >= (3, 11):
        import tomllib
    else:
        try:
            import tomli as tomllib
        except ImportError:
            from setuptools._vendor import tomli as tomllib

    if not os.path.isfile(tomlfilepath):
        raise FileNotFoundError(f"Configuration file {tomlfilepath!r} does not exist.")

    dependencies = []
    with open(tomlfilepath, "rb") as file:
        asdict = tomllib.load(file) or {}
        dependencies.extend(asdict.get("build-system", {}).get("requires", []))
        dependencies.extend(asdict.get("project", {}).get("dependencies", []))
    
    return set([extract_package_name(d) for d in dependencies])

def clean_dst_directory_for_cpp(base_dir, package_name, cpp_module_name=None):
    # Cleans destination package directory. Build directory is kept as-is.
    # This is for cleaning auto-generated files by CMake install command.
    module_dir=os.path.join(base_dir, package_name)
    module_include_dir=os.path.join(module_dir, "include")
    shutil.rmtree(module_include_dir,True)
    module_third_party_dir=os.path.join(module_dir, "thirdParty")
    shutil.rmtree(module_third_party_dir,True)
    if cpp_module_name is None:
        cpp_module_name = package_name
    for f in glob.glob(os.path.join(module_dir,"share/cmake",package_name+"*Targets*.cmake"),recursive=True):
        os.remove(f)
    for f in glob.glob(os.path.join(module_dir,"**/*"+cpp_module_name+"*.*"),recursive=True):
        if os.path.splitext(f)[1] in [".so",".lib",".dll",".dll.a",".pyd"]:
            os.remove(f)

class ASRCConfigParserMixIn:
    def generate_build_identifier(self):
        return "B"+datetime.datetime.now().strftime("%Y%m%d%H%M%S")
    def is_pure_python_asrc_plugin(self):
        # TODO より適切な判定方式にする
        return not (
            len(glob.glob(os.path.join(self._asrc_base_dir,"builder*.*"))) > 0 and
            os.path.exists(os.path.join(self._asrc_base_dir,"CMakeLists.txt"))
        )

    def parse_asrc_config(self):
        self._asrc_base_dir = self.distribution.src_root or os.curdir
        self._asrc_package_name = self.distribution.name or find_packages(os.curdir)[0].partition('.')[0]
        self._asrc_is_core_plugin = False
        self._asrc_enable_build_identification = False
        self._asrc_core_plugins = None
        self._asrc_user_plugins = None
        if sys.version_info >= (3, 11):
            import tomllib
        else:
            try:
                import tomli as tomllib
            except ImportError:
                from setuptools._vendor import tomli as tomllib
        if sys.version_info < (3, 10):
            from importlib_metadata import entry_points
        else:
            from importlib.metadata import entry_points

        _, tomlfiles = self.distribution._get_project_config_files(None)

        dependencies = []

        for filename in tomlfiles:
            filepath = os.path.abspath(filename)
            if not os.path.isfile(filepath):
                raise FileNotFoundError(f"Configuration file {filepath!r} does not exist.")
            with open(filepath, "rb") as file:
                asdict = tomllib.load(file) or {}
                tool_table = asdict.get("tool", {})
                asrc_table = tool_table.get("ASRCAISim1", {})
                dependencies.extend(asdict.get("build-system", {}).get("requires", []))
                dependencies.extend(asdict.get("project", {}).get("dependencies", []))
            
            self._asrc_is_core_plugin = asrc_table.get("is-core-plugin", self._asrc_is_core_plugin)
            self._asrc_enable_build_identification = asrc_table.get("enable-build-identification", self._asrc_enable_build_identification)
            self._asrc_source_encoding = asrc_table.get("source-encoding", "utf-8")

        dependencies = list(set([extract_package_name(d) for d in dependencies]))

        self._asrc_core_plugins = list(set(
            ep.name for ep in entry_points(group='ASRCAISim1.core_plugins')
            if ep.name in dependencies
        ))
        self._asrc_cpp_core_plugins = list(set(
            ep.name for ep in entry_points(group='ASRCAISim1.cpp_core_plugins')
            if ep.name in self._asrc_core_plugins
        ))

        self._asrc_user_plugins = list(set(
            ep.name for ep in entry_points(group='ASRCAISim1.user_plugins')
            if ep.name in dependencies
        ))
        self._asrc_cpp_user_plugins = list(set(
            ep.name for ep in entry_points(group='ASRCAISim1.cpp_user_plugins')
            if ep.name in self._asrc_user_plugins
        ))

        # validation
        if self._asrc_is_core_plugin:
            if self._asrc_enable_build_identification:
                raise OptionError(f"A core plugin cannot enable build identification. Set enable-build-identification=true in your toml file.")
            if self._asrc_user_plugins:
                raise OptionError(f"A core plugin cannot have dependency on user plugins.")
        if self._asrc_package_name in self._asrc_core_plugins:
            raise OptionError(f"You gave package name itself as dependency (core-plugins).")
        if self._asrc_package_name in self._asrc_user_plugins:
            raise OptionError(f"You gave package name itself as dependency (user-plugins).")

        if self._asrc_enable_build_identification:
            c=os.path.join(self._asrc_base_dir, self._asrc_package_name,"BuildIdentifier.txt")
            if os.path.exists(c):
                # Already defined.
                with open(c,"r",encoding=self._asrc_source_encoding) as reader:
                    self._asrc_build_identifier=reader.readline().strip()
            else:
                self._asrc_build_identifier=self.generate_build_identifier()
                # You can designate the build identifier manually. You may not use dot(".") in the build identifier.
                #self._asrc_build_identifier="B_Manual_v_1_0_0"
            content=self._asrc_build_identifier
            with open(c,"w",encoding=self._asrc_source_encoding) as writer:
                writer.write(content)

class egg_info(egg_info_orig, ASRCConfigParserMixIn):
    ''' (1) ASRCAISim1.{core|user}_pluginsグループに対象pluginのEntryPointを自動的に追加する。
            Add an EntryPoint for the plugin to ASRCAISim1.{core|user}_plugins group.
    '''
    def run(self):
        self.parse_asrc_config()
        # Add entry_points
        if self._asrc_is_core_plugin:
            group = 'ASRCAISim1.core_plugins'
        else:
            group = 'ASRCAISim1.user_plugins'
        if self.distribution.entry_points is None:
            self.distribution.entry_points = {}
        if not group in self.distribution.entry_points:
            self.distribution.entry_points[group] = []
        self.distribution.entry_points[group].append(
            self._asrc_package_name + ' = ' + self._asrc_package_name
        )
        if not self.is_pure_python_asrc_plugin():
            if self._asrc_is_core_plugin:
                group = 'ASRCAISim1.cpp_core_plugins'
            else:
                group = 'ASRCAISim1.cpp_user_plugins'
            if not group in self.distribution.entry_points:
                self.distribution.entry_points[group] = []
            self.distribution.entry_points[group].append(
                self._asrc_package_name + ' = ' + self._asrc_package_name
            )
        egg_info_orig.run(self)

class sdist(sdist_orig, ASRCConfigParserMixIn):
    ''' (1) MANIFEST.in(.self)の使い回しを可能とするため、ファイル中の"{packageName}"をpackage名に置換する。
            To enable copy & paste of MANIFEST.in(.self) among plugins, this custom command converts "{packageName}" in the file into package name automatically.
    '''
    def run(self):
        self.parse_asrc_config()

        manifest=open("MANIFEST.in","w",encoding=self._asrc_source_encoding)
        with open("MANIFEST.in.self","r",encoding=self._asrc_source_encoding) as fragment:
            manifest.write(fragment.read().replace("{packageName}",self._asrc_package_name)+"\n")
        manifest.close()
        if not self.is_pure_python_asrc_plugin():
            clean_dst_directory_for_cpp(self._asrc_base_dir, self._asrc_package_name)
        sdist_orig.run(self)

class build(build_orig, ASRCConfigParserMixIn):
    ''' (1) MANIFEST.in(.self)の使い回しを可能とするため、ファイル中の"{packageName}"をpackage名に置換する。
            To enable copy & paste of MANIFEST.in(.self) among plugins, this custom command converts "{packageName}" in the file into package name automatically.

        (2) 依存しているプラグインを列挙し、必要に応じそれらのbuild identifierを記録する。
            Enumeate depending plugins and record their build identifiers if needed.

        (3) C++部分があればビルドを行う。(現バージョンではbuild_extを使用せず、buildでビルドする。)
            If the plugin has C++ part, build it. (In the current version, we don't use build_ext command.)
    '''
    def run(self):
        self.parse_asrc_config()

        manifest=open("MANIFEST.in","w",encoding=self._asrc_source_encoding)
        with open("MANIFEST.in.self","r",encoding=self._asrc_source_encoding) as fragment:
            manifest.write(fragment.read().replace("{packageName}",self._asrc_package_name)+"\n")
        manifest.close()

        # check plugin dependency
        os.environ['ASRCAISim1_disable_automatic_import_of_core_plugins'] = 'True'
        import ASRCAISim1
        import importlib
        dep_plugins = {}
        for core_plugin in self._asrc_core_plugins:
            dep_plugins[core_plugin] = importlib.import_module(core_plugin)
        for user_plugin in self._asrc_user_plugins:
            dep_plugins[user_plugin] = importlib.import_module(user_plugin)

        import json
        dep_info_file = os.path.join(self._asrc_base_dir, self._asrc_package_name, "PluginDependencyInfo.json")
        if os.path.exists(dep_info_file):
            os.remove(dep_info_file)

        dep_info = {}
        for dep_name, dep_plugin in dep_plugins.items():
            if self._asrc_enable_build_identification:
                dep_info[dep_name] = ASRCAISim1.getBuildIdentifier(dep_plugin)
            else:
                dep_info[dep_name] = ''
        with open(dep_info_file, 'w') as f:
            json.dump(dep_info, f, indent='    ')

        # build C++ part
        if not self.is_pure_python_asrc_plugin():
            self.debug = os.getenv('ASRCAISim1_enable_debug_build', 'False').lower() in ('true', '1', 't')
            self.build_config = "Debug" if self.debug else "Release"
            self.msys = os.getenv('ASRCAISim1_build_with_msys', 'False').lower() in ('true', '1', 't')

            clean_dst_directory_for_cpp(self._asrc_base_dir, self._asrc_package_name)
            import sys
            import subprocess
            py_ver="%d.%d" % sys.version_info[:2]
            os.environ["PythonLibsNew_FIND_VERSION"]=py_ver
            os.environ["Python_ROOT_DIR"]=os.path.dirname(sys.executable)
            os.environ["Python3_ROOT_DIR"]=os.path.dirname(sys.executable)
            extra_cmake_prefix_path = ";".join(
                [
                    path for path in 
                    [ASRCAISim1.get_cmake_dir()] + [(p.get_cmake_dir() if hasattr(p,"get_cmake_dir") else "") for p in dep_plugins.items()]
                    if path != ""
                ]
            )

            if(os.name=="nt"):
                if(self.msys):
                    base_cmd_args = ["bash","./builder.sh"]
                else:
                    base_cmd_args = [".\\builder.bat"]
            else:
                base_cmd_args = ["bash", "./builder.sh"]

            common_cmd_args = [
                self.build_config,
                self._asrc_package_name,
                extra_cmake_prefix_path,
            ]
            if(os.name=="nt"):
                common_cmd_args = [
                    c.replace(";","^;")
                    for c in common_cmd_args
                ]

            cmd_args = base_cmd_args + common_cmd_args
            subprocess.check_call(cmd_args)
            if self._asrc_enable_build_identification:
                cmd_args = cmd_args + [self._asrc_build_identifier]
                subprocess.check_call(cmd_args)

            if(os.name=="nt"):
                dummy=os.path.join(self._asrc_base_dir,self._asrc_package_name+"/lib"+self._asrc_package_name+".pyd")
                if(os.path.exists(dummy)):
                    os.remove(dummy)
                f=open(dummy,"w",encoding=self._asrc_source_encoding) #Dummy
                f.close()
                if self._asrc_enable_build_identification:
                    dummy=os.path.join(self._asrc_base_dir,self._asrc_package_name+"/lib"+self._asrc_package_name+self._asrc_build_identifier+".pyd")
                    if(os.path.exists(dummy)):
                        os.remove(dummy)
                    f=open(dummy,"w",encoding=self._asrc_source_encoding) #Dummy
                    f.close()

            # replace namespace declaration in installed headers
            for header in glob.glob(
                os.path.join(self._asrc_base_dir, self._asrc_package_name+"/include/**"),recursive=True):
                if (not os.path.exists(header)) or os.path.isdir(header):
                    continue
                with open(header,"r",encoding=self._asrc_source_encoding) as reader:
                    content = reader.read()
                if self._asrc_enable_build_identification and self._asrc_build_identifier in header:
                    content = re.sub(
                        "ASRC_PLUGIN_NAMESPACE_BEGIN",
                        "namespace "+self._asrc_package_name+" {\n"+
                        "inline namespace "+self._asrc_build_identifier+" {",
                        content
                    )
                    content = re.sub(
                        "ASRC_PLUGIN_NAMESPACE_END",
                        "}}",
                        content
                    )
                    content = re.sub(
                        "ASRC_PLUGIN_NAMESPACE::",
                        self._asrc_package_name+"::"+self._asrc_build_identifier+"::",
                        content
                    )
                else:
                    content = re.sub(
                        "ASRC_PLUGIN_NAMESPACE_BEGIN",
                        "namespace "+self._asrc_package_name+" {",
                        content
                    )
                    content = re.sub(
                        "ASRC_PLUGIN_NAMESPACE_END",
                        "}",
                        content
                    )
                    content = re.sub(
                        "ASRC_PLUGIN_NAMESPACE::",
                        self._asrc_package_name+"::",
                        content
                    )
                content = re.sub(
                    "ASRC_PLUGIN_BII_NAMESPACE_BEGIN",
                    "namespace "+self._asrc_package_name+" {",
                    content
                )
                content = re.sub(
                    "ASRC_PLUGIN_BII_NAMESPACE_END",
                    "}",
                    content
                )
                content = re.sub(
                    "ASRC_PLUGIN_BII_NAMESPACE::",
                    self._asrc_package_name+"::",
                    content
                )
                content = re.sub(
                    r"#(el)?if\s*(def)?(ined)?[\s\(]*ASRC_BUILD_PLUGIN_BII_PART(\)|[^\S\r\n])*([\r\n]*)",
                    r"#\1if 0 // automatic replacement of #ifdef ASRC_BUILD_PLUGIN_BII_PART\5",
                    content
                )
                content = re.sub(
                    r"#(el)?if\s*[n|!]?\s*(def)?(ined)?[\s\(]*ASRC_BUILD_PLUGIN_BII_PART(\)|[^\S\r\n])*([\r\n]*)",
                    r"#\1if 1 // automatic replacement of #ifndef ASRC_BUILD_PLUGIN_BII_PART\5",
                    content
                )
                with open(header,"w",encoding=self._asrc_source_encoding) as writer:
                    writer.write(content)

        build_orig.run(self)

class install(install_orig, ASRCConfigParserMixIn):
    ''' (1) C++部分があれば、ビルドの後始末を行う。
            If the plugin has C++ part, perform the post-process after build.
    '''
    def run(self):
        self.parse_asrc_config()
        install_orig.run(self)
        if not self.is_pure_python_asrc_plugin():
            if(os.name=="nt"):
                dummy=os.path.join(self._asrc_base_dir,self._asrc_package_name+"/lib"+self._asrc_package_name+".pyd")
                if(os.path.exists(dummy)):
                    os.remove(dummy) #Dummy

class clean(clean_orig, ASRCConfigParserMixIn):
    ''' (1) 標準外のファイルもビルド時に生成されるため、それらを独自に削除する。
            Remove out-of-standard files generated by build process.
    '''
    def run(self):
        self.parse_asrc_config()
        clean_orig.run(self)

        build_dir=os.path.join(self._asrc_base_dir, "build")
        dist_dir=os.path.join(self._asrc_base_dir, "dist")
        shutil.rmtree(build_dir,True)
        for dist in glob.glob(os.path.join(dist_dir, self._asrc_package_name+"-*")):
            os.remove(dist)
        files=[
            os.path.join(self._asrc_base_dir,"MANIFEST.in"),
            os.path.join(self._asrc_base_dir, self._asrc_package_name, "BuildIdentifier.txt"),
            os.path.join(self._asrc_base_dir, self._asrc_package_name, "PluginDependencyInfo.json"),
        ]
        for f in files:
            if os.path.exists(f):
                os.remove(f)
        shutil.rmtree(os.path.join(self._asrc_base_dir,self._asrc_package_name+".egg-info"),True)
        for d in glob.glob(os.path.join(self._asrc_base_dir,"**/__pycache__"),recursive=True):
            shutil.rmtree(d,True)
        if not self.is_pure_python_asrc_plugin():
            clean_dst_directory_for_cpp(self._asrc_base_dir, self._asrc_package_name)
        msys = os.getenv('ASRCAISim1_build_with_msys', 'False').lower() in ('true', '1', 't')
        if(os.name=="nt" and not msys):
            cleaner=os.path.join(self._asrc_base_dir, "thirdParty","scripts","cleaner.bat")
            if os.path.exists(cleaner):
                import subprocess
                subprocess.check_call([
                    cleaner,
                ])
        else:
            cleaner=os.path.join(self._asrc_base_dir, "thirdParty","scripts","cleaner.sh")
            if os.path.exists(cleaner):
                import subprocess
                subprocess.check_call([
                    "bash",
                    cleaner,
                ])
