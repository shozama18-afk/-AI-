// Copyright (c) 2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
/**
 * @file
 * @brief Entityをデータ型として持つ木構造の一般化。boost::property_treeを使用している。
 */
#pragma once
#include <boost/property_tree/ptree.hpp>
#include <cereal/cereal.hpp>
#include <cereal/details/util.hpp>
#include <filesystem>
#include <map>
#include <set>
#include <ranges>
#include "macros/common_macros.h"
#include "../py_init_hook.h"
#include "../traits/converter.h"
#include "range_iterable.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
ASRC_INLINE_NAMESPACE_BEGIN(util)

/**
 * @brief shared_ptrを値として持つ名前付き木構造を表現する構造体。
 * 
 * @tparam _Instance			保持対象の要素の型
 * @tparam _Identifier			要素を識別可能な識別子の型
 * @tparam _IdentifierGetter	要素から識別子を得る関数オブジェクトの型
 * @tparam _Stringifier			要素を文字列化する関数オブジェクトの型
 * @tparam asWeak				shared_ptrの代わりにweak_ptrで保持するかどうか。
 * 
 * @note 木構造のパスはファイルパスと同様に"/"区切りとし、"."や".."に対応している。
 * 
 */
template<
	class _Instance,
	class _Identifier,
	traits::converter<const std::shared_ptr<const _Instance>&,_Identifier> _IdentifierGetter,
	traits::stringifier<const std::shared_ptr<const _Instance>&> _Stringifier,
	bool asWeak=false
>
struct PYBIND11_EXPORT ComponentTree {
    using Instance = _Instance; //!< 保持対象の要素の型
	using Identifier = _Identifier; //!< 要素を識別可能な識別子の型
	using IdentifierGetter = _IdentifierGetter; //!< 要素から識別子を得る関数オブジェクトの型
    using Stringifier = _Stringifier; //!< 要素を文字列化する関数オブジェクトの型
    using Data = std::conditional_t<asWeak,std::weak_ptr<Instance>,std::shared_ptr<Instance>>; //!< 要素を保持するデータ型。shared_ptrまたはweak_ptrになる。
    using Tree=boost::property_tree::basic_ptree<std::string,Data>; //!< 木構造の型
    using Path=typename Tree::path_type; //!< 木構造の要素を辿るパスの型。std::stringになる。

    ComponentTree(bool _forceRootAsWeak=true):forceRootAsWeak(_forceRootAsWeak){}

	/**
	 * @brief 指定したパスが存在するかどうかを返す。
	 * 
	 * @param [in] _path 調べる対象のパス
	 */
    bool contains(const std::string& _path) const{
        std::string path=resolve(_path);
        auto opt=tree.get_child_optional(Path(path,'/'));
        if(opt.has_value()){
            std::shared_ptr<Instance> instance;
            if constexpr (asWeak){
                auto wPtr=opt.value().data();
                if(wPtr.expired()){
                    instance=nullptr;
                }
                instance=wPtr.lock();
            }else{
                if(forceRootAsWeak && path==""){
                    instance=rootAsWeak.lock();
                }else{
                    instance=opt.value().data();
                }
            }
            if(instance){
                return true;
            }else{
                // putで明示的に追加していないnullptrノードは存在しないものとみなす。
                return forward_paths.contains(path);
            }
        }else{
            return false;
        }
    }


	/**
	 * @brief 要素が存在するかどうかを返す。
	 * 
	 * @param [in] instance 調べる対象のインスタンス
	 */
    bool contains(const std::shared_ptr<Instance>& instance) const{
        if(!instance){
            // nullptrからの逆引きは不可。例外ではなく、つねにfalseを返すものとする。
            return false;
        }
        auto eid=identify(instance);
        return reverse_map.contains(eid);
    }

	/**
	 * @brief 指定したパスに要素を追加する。パスの途中に要素が存在しなければnullptrで埋める。
	 * 
	 * @param [in] _path	追加先のパス。
	 * @param [in] instance	追加対象の要素
	 */
    void put(const std::string& _path, const std::shared_ptr<Instance>& instance){
        std::string path=resolve(_path);

        if(contains(path)){
            throw std::runtime_error("There is already a path '"+path+"' in the tree. To replace, remove the old node first.");
        }

        if(instance){
            if(contains(instance)){
                throw std::runtime_error("There is already an instance '"+stringify(instance)+" at '"+get_path(instance)+"' in the tree. To replace, remove the old node first.");
            }
            auto eid=identify(instance);
            reverse_map[eid]=path;
        }else{
            null_paths.insert(path);
        }
        forward_paths.insert(path);

        // 間のノードが存在しない場合はnullptrで埋める
        auto slash=path.rfind("/");
        if(slash!=std::string::npos){
            std::string sub=path.substr(0,slash);
            if(!contains(sub)){
                put(sub,nullptr);
            }
        }

        if constexpr (asWeak){
            tree.put(Path(path,'/'),Data(instance));
        }else{
            if(forceRootAsWeak && path==""){
                rootAsWeak=instance;
            }else{
                tree.put(Path(path,'/'),instance);
            }
        }
    }

	/**
	 * @brief 指定したパスの要素を削除する。削除した要素の子要素も全て削除される。
	 * 
	 * @param [in] _path	削除対象のパス。
	 */
    void remove(const std::string& _path){
        std::string path=resolve(_path);
        if(contains(path)){
            if(forceRootAsWeak && path==""){
                rootAsWeak.reset();
            }
            auto t=tree.get_child(Path(path,'/'));
            remove_sub(t,path);
            auto slash=path.rfind("/");
            if(slash==std::string::npos){
                tree.erase(path);
            }else{
                auto& parent=tree.get_child(Path(path.substr(0,slash),'/'));
                parent.erase(path.substr(slash+1));
            }
        }
    }

	/**
	 * @brief 要素を削除する。削除した要素の子要素も全て削除される。
	 * 
	 * @param [in] instance	削除対象の要素
	 */
    void remove(const std::shared_ptr<Instance>& instance){
        if(contains(instance)){
            remove(get_path(instance));
        }
    }

	/**
	 * @brief 指定したパスにある要素を取得する。
	 */
    std::shared_ptr<Instance> get(const std::string& _path) const{
        std::string path=resolve(_path);
        auto opt=tree.get_child_optional(Path(path,'/'));
        if(opt.has_value()){
            if constexpr (asWeak){
                auto wPtr=opt.value().data();
                if(wPtr.expired()){
                    return nullptr;
                }
                return wPtr.lock();
            }else{
                if(forceRootAsWeak && path==""){
                    return rootAsWeak.lock();
                }else{
                    return opt.value().data();
                }
            }
        }else{
            return nullptr;
        }
    }

	/**
	 * @brief 指定した要素のノード名を取得する。
	 */
    std::string get_name(const std::shared_ptr<Instance>& instance) const{
        if(!instance){
            throw std::runtime_error("ComponentTree::get_name cannot accept nullptr.");
        }
        auto eid=identify(instance);
        auto found=reverse_map.find(eid);
        if(found==reverse_map.end()){
            throw std::runtime_error("ComponentTree::get_name failed. '"+stringify(instance)+"' is not in the tree.");
        }
		std::string ret=found->second;
		auto slash=ret.rfind("/");
		if(slash!=std::string::npos){
			ret=ret.substr(slash+1);
		}
        return std::move(ret);
    }

	/**
	 * @brief 指定した要素のパスを取得する。
	 */
    std::string get_path(const std::shared_ptr<Instance>& instance) const{
        if(!instance){
            throw std::runtime_error("ComponentTree::get_path cannot accept nullptr.");
        }
        auto eid=identify(instance);
        auto found=reverse_map.find(eid);
        if(found==reverse_map.end()){
            throw std::runtime_error("ComponentTree::get_path failed. '"+stringify(instance)+"' is not in the tree.");
        }
        return found->second;
    }

	/**
	 * @brief 指定したパスの下にある子要素のrangeを取得する。
	 * 
	 * @note 再帰ではなく直下の子要素のみ
	 * 
	 * @return std::pair<std::string,std::shared_ptr<Instance>>を要素とするstd::ranges::range
	 */
    auto get_children(const std::string& _path) const{
        std::string path=resolve(_path);
        if(!contains(path)){
            throw std::runtime_error("There is no node at '"+path+"' in the tree.");
        }
        auto& child=tree.get_child(Path(path,'/'));

        return (
            std::ranges::subrange(child.begin(),child.end())
            | std::views::transform([this](auto&& v) -> std::pair<std::string,std::shared_ptr<Instance>> {
                if constexpr (asWeak){
                    return {v.first,v.second.data().lock()};
                }else{
                    return {v.first,v.second.data()};
                }
            })
        );
    }

	/**
	 * @brief 指定した要素の下にある子要素のrangeを取得する。
	 * 
	 * @note 再帰ではなく直下の子要素のみ
	 * 
	 * @return std::pair<std::string,std::shared_ptr<Instance>>を要素とするstd::ranges::range
	 */
    auto get_children(const std::shared_ptr<Instance>& instance) const{
        return get_children(get_path(instance));
    }

	/**
	 * @brief 指定したパスを正規化する。ただし、"."と".."は""とする。
	 * 
	 * @details 連続する"/"を一つにし、"."と".."による階層移動を取り除き、先頭と末尾の"/"を取り除く。
	 * 			"/a/b/./../////c//d/" -> "a/b/c/d"
     * 
     *          なお、本来の正規化では"."と".."はそのままになるが、ここでは""としてルートを表すものとする。
	 * 
	 */
    static std::string resolve(const std::string& path){
        auto ret=std::filesystem::path(path).lexically_normal().generic_string();
        if(ret=="." || ret==".."){
            return "";
        }
        if(ret.size()>0 && ret.at(ret.size()-1)=='/'){
            ret=ret.substr(0,ret.size()-1);
        }
        if(ret.size()>0 && ret.at(0)=='/'){
            ret=ret.substr(1);
        }
        return std::move(ret);
    }

	/**
	 * @brief cerealによるシリアライズを行う。
	 */
    template<class Archive>
    void save(Archive & archive) const{
		std::map<std::string,std::shared_ptr<Instance>> instances;
		for(auto&& [eid,path] : reverse_map){
			instances[path]=get(path);
		}

        archive(
            CEREAL_NVP(forceRootAsWeak),
            CEREAL_NVP(rootAsWeak),
            CEREAL_NVP(instances),
            CEREAL_NVP(null_paths)
		);
	}

	/**
	 * @brief cerealによるデシリアライズを行う。
	 */
    template<class Archive>
    void load(Archive & archive){
        reverse_map.clear();
        forward_paths.clear();
        null_paths.clear();
		std::map<std::string,std::shared_ptr<Instance>> instances;
        std::set<std::string> _null_paths;

        archive(
            CEREAL_NVP(forceRootAsWeak),
            CEREAL_NVP(rootAsWeak),
            CEREAL_NVP(instances),
            cereal::make_nvp("null_paths",_null_paths)
		);

		for(auto&& [path,instance] : instances){
			put(path,instance);
		}
		for(auto&& path : _null_paths){
            if(!contains(path)){
			    put(path,nullptr);
            }
		}
	}

	/**
	 * @brief 木構造を標準出力に書き出す。
	 */
    void print() const{
        print_sub(tree,0);
        std::cout<<std::endl;
    }

    private:
	IdentifierGetter identify; //!< Instanceを識別するIdentifierを得るための関数オブジェクト
    Stringifier stringify; //!< Instanceを文字列化する関数オブジェクト
    Tree tree; //!< Instanceのproperty_tree
    bool forceRootAsWeak; //!< ルートノードをweak_ptrで保持することを強制するかどうか。falseならTreeのdataでなくrootAsWeakにルートのweak_ptrを保持する。
    std::weak_ptr<Instance> rootAsWeak; //!< ルートノードのweak_ptr。ルート自身がメンバにTreeを持つ場合の循環参照を避けるためのもの。
	std::map<Identifier,std::string> reverse_map; //!< Identifierからpathへのマッピング
    std::set<std::string> forward_paths; //!< putで明示的に追加されたpathのset
    std::set<std::string> null_paths; //!< nullptrが入っているpathのset

    void remove_sub(const Tree& t,const std::string& _path){
        for(auto&& [key, child] : t){
            std::string path = _path.empty() ? key : _path+"/"+key;
            remove_sub(child,path);
        }
        std::shared_ptr<Instance> instance;
        if constexpr (asWeak){
            if(t.data().expired()){
                instance=nullptr;
            }else{
                instance=t.data().lock();
            }
        }else{
            if(forceRootAsWeak && _path==""){
                instance=rootAsWeak.lock();
            }else{
                instance=t.data();
            }
        }
        if(instance){
            reverse_map.erase(reverse_map.find(identify(instance)));
        }else{
            null_paths.erase(_path);
        }
        forward_paths.erase(_path);
    }

    static std::string indent(int level){
        std::string s;
        for(int i=0;i<level;++i){
            s+="  ";
        }
        return s;
    }

    void print_sub(const Tree& t, int level) const{
		if constexpr (asWeak){
        	std::cout<<"('"<<stringify(t.data().lock())<<"'){";
		}else{
            if(level==0 && forceRootAsWeak){
        	    std::cout<<"('"<<stringify(rootAsWeak.lock())<<"'){";
            }else{
        	    std::cout<<"('"<<stringify(t.data())<<"'){";
            }
		}

        bool first=true;
        for(auto&& [key, child] : t){
            if(!first){
                std::cout<<",";
            }
            std::cout<<std::endl;
            std::cout<<indent(level+1)<<"'"<<key<<"':";
            print_sub(child,level+1);
            first=false;
        }
        if(!first){
            std::cout<<std::endl;
            std::cout<<indent(level);
        }
        std::cout<<"}";
    }
};
ASRC_NAMESPACE_END(util)

ASRC_NAMESPACE_BEGIN(traits)
template<class T>
struct is_component_tree : std::false_type {};

template<
	class Instance,
	class Identifier,
	converter<const std::shared_ptr<const Instance>&,Identifier> IdentifierGetter,
	stringifier<const std::shared_ptr<const Instance>&> Stringifier,
	bool asWeak
>
struct is_component_tree<util::ComponentTree<Instance,Identifier,IdentifierGetter,Stringifier,asWeak>> : std::true_type {};

template<class T>
concept component_tree = is_component_tree<T>::value;

ASRC_NAMESPACE_END(traits)

ASRC_INLINE_NAMESPACE_BEGIN(util)

template<traits::component_tree TreeType, typename ... Extras>
pybind11::class_<TreeType> expose_component_tree_name(pybind11::module &m,const std::string& className, Extras&& ... extras){

    using namespace pybind11::literals;
    using ChildrenRangeType = decltype(std::declval<TreeType>().get_children(""));

	expose_range<ChildrenRangeType,Extras...>(m,std::forward<Extras>(extras)...);	

	using Instance = typename TreeType::Instance;

	try{
		return pybind11::class_<TreeType>(m,className.c_str(),std::forward<Extras>(extras)...)
		.def(py_init<bool>(),"_forceRootAsWeak"_a=true)
		.def("contains",pybind11::overload_cast<const std::string&>(&TreeType::contains,pybind11::const_))
		.def("contains",pybind11::overload_cast<const std::shared_ptr<Instance>&>(&TreeType::contains,pybind11::const_))
		.def("put",&TreeType::put)
		.def("remove",pybind11::overload_cast<const std::string&>(&TreeType::remove))
		.def("remove",pybind11::overload_cast<const std::shared_ptr<Instance>&>(&TreeType::remove))
		.def("get",&TreeType::get)
		.def("get_name",&TreeType::get_name)
		.def("get_path",&TreeType::get_path)
		.def("get_children",[](const TreeType& v, const std::string& _path){return v.get_children(_path);},pybind11::return_value_policy::reference_internal)
		.def("get_children",[](const TreeType& v, const std::shared_ptr<Instance>& instance){return v.get_children(instance);},pybind11::return_value_policy::reference_internal)
		.def("print",&TreeType::print)
		.def_static("resolve",&TreeType::resolve)
		;
    }catch(std::exception& ex){
        if(pybind11::detail::get_type_info(typeid(TreeType))){
            // already bound
            auto ret=pybind11::reinterpret_borrow<pybind11::class_<TreeType>>(pybind11::detail::get_type_handle(typeid(TreeType),true));
            assert(ret);
            return std::move(ret);
        }else{
            throw ex;
        }
	}
}

template<traits::component_tree TreeType, typename ... Extras>
pybind11::class_<TreeType> expose_component_tree(pybind11::module &m, Extras&& ... extras){
	return expose_component_tree_name<TreeType,Extras...>(m,cereal::util::demangle(typeid(TreeType).name()),std::forward<Extras>(extras)...);
}

ASRC_NAMESPACE_END(util)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
