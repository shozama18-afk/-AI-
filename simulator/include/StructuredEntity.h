// Copyright (c) 2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
/**
 * @file
 * @brief 他のEntityを名前付きcomponentとして木構造で保持する
 * 
 */
#pragma once
#include "Common.h"
#include <memory>
#include <random>
#include <set>
#include <pybind11/pybind11.h>
#include <nlohmann/json.hpp>
#include "Entity.h"
#include "Utility.h"
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

class PlaceHolderEntity; // forward declaration

/**
 * @brief 名前付きの木構造で子要素を保持可能なタイプの基底クラス。
 * 
 * @details
 * 
 *      managedなtreeとunmanagedなtreeに分けて扱う。
 *      前者は、SimulationManagerによって管理され、自動的に周期処理が呼び出される。
 *      後者は、treeのrootにあるStructuredEntityが寿命や処理の呼び出しを管理する。
 * 
 *      managedなStructuredEntityは、managedなtreeに属すると同時に、自身をrootとするunmanagedなtreeを持つことができる。
 *      unmanagedなStructuredEntityは、unmanagedなtreeのみに属する。
 * 
 *      component tree上のパスは'/'区切りかつ'/'で始まる文字列とする。
 *      「自分自身のパス」はmanagedなStructuredEntityにおいてはmanaged tree上のパス、unmanagedなStructuredEntityにおいてはunmanaged tree上のパスとなる。
 * 
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(StructuredEntity,Entity)
    public:
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;

    bool isRoot(bool inManagedTree) const; //!< 自身が他者のcomponentではない(component treeのルートである)かどうかを返す。
    bool isComponent(bool inManagedTree) const; //!< 自身が他者のcomponentである(component treeのルートでない)かどうかを返す。
    std::string getComponentName(bool inManagedTree) const; //!< 自身が属するcomponent treeにおける自身のcomponent名を返す。
    std::string getComponentPath(bool inManagedTree) const; //!< 自身が属するcomponent treeにおける自身の絶対パスを返す。'/'区切りかつ'/'で始まる。

    /**
     * @brief 自身が属するcomponent treeの最上位のStructuredEntityをTにcastして返す。
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    std::shared_ptr<T> root(bool inManagedTree) const{
        if(isRoot(inManagedTree)){
            return non_const_this<T>();
        }else{
            if(inManagedTree && !isManaged()){
                throw std::runtime_error("StructuredEntity::root(true) failed. Unmanaged StructuredEntity cannot belong to a managed tree.");
            }
            if(_root.expired()){
                throw std::runtime_error("The root of "+getFullName()+" is expired!.");
            }
            auto casted=std::dynamic_pointer_cast<T>(_root.lock());
            if(!casted){
                throw std::runtime_error("'"+_root.lock()->getFullName()+"' is not an instance of '"+cereal::util::demangle(typeid(T).name())+"', but '"+_root.lock()->getDemangledName()+"'.");
            }
            return std::move(casted);
        }
    }

    /**
     * @brief 自身が属するcomponent treeの最上位のStructuredEntityが既に消去済(expired)かどうかを返す。自身がrootならばfalseを返す。
     */
    bool isRootExpired(bool inManagedTree) const;

    /**
     * @brief 自身が属するcomponent treeの直上のStructuredEntityをTにcastして返す。
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    std::shared_ptr<T> holder(bool inManagedTree) const{
        if(isRoot(inManagedTree)){
            return nullptr;
        }else{
            if(inManagedTree && !isManaged()){
                throw std::runtime_error("StructuredEntity::holder(true) failed. Unmanaged StructuredEntity cannot belong to a managed tree.");
            }
            if(_holder.expired()){
                throw std::runtime_error("The holder of "+getFullName()+" is expired!.");
            }
            auto casted=std::dynamic_pointer_cast<T>(_holder.lock());
            if(!casted){
                throw std::runtime_error("'"+_holder.lock()->getFullName()+"' is not an instance of '"+cereal::util::demangle(typeid(T).name())+"', but '"+_holder.lock()->getDemangledName()+"'.");
            }
            return std::move(casted);
        }
    }

    /**
     * @brief 自身が属するcomponent treeの直上のStructuredEntityが既に消去済(expired)かどうかを返す。元々存在しない場合はtrueを返す。
     */
    bool isHolderExpired(bool inManagedTree) const;

    /**
     * @brief 自身が属するcomponent treeの直上のStructuredEntityをTにcastして返す。
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    std::shared_ptr<T> non_const_this() const{
        auto casted=std::dynamic_pointer_cast<T>(std::const_pointer_cast<Entity>(shared_from_this()));
        if(!casted){
            throw std::runtime_error("'"+getFullName()+"' is not an instance of '"+cereal::util::demangle(typeid(T).name())+"', but '"+getDemangledName()+"'.");
        }
        return std::move(casted);
    }

    /**
     * @brief componentを生成し、Tにcastして返す。
     * 
     * @param [in] inManagedTree        trueにするとmanaged treeに、falseにするとunmanaged treeに生成する。
     * @param [in] newComponentName     componentの名称。生成者のfullName+"/"+newComponentNameが新たなcomponentのfullNameとなる。
     * @param [in] newComponentConfig   componentの生成用config。createEntityを単一のjsonで呼び出すときと同じフォーマット。一部の項目は自動的に上書きされる。
     * 
     * @note managed componentは自身もmanagedのときのみ生成可能。
     * @note makeChildren()の中で呼び出すのが基本的な使い方。
     * @note unmanaged componentを生成する場合、isEpisodic, firstTick, intervalは自身と同じになるよう上書きされる。
     * 
     * @code
     *  nl::json c={
     *      {"baseName","PhysicalAsset"},
     *      {"className","Class"},
     *      {"modelName","Model"},
     *      {"modelConfig",modelConfig},
     *      {"instanceConfig",instanceConfig}
     *  };
     * 
     *  auto comp1=createComponent(true,"comp1",c);
     * @endcode
     * 
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    std::shared_ptr<T> createComponent(bool inManagedTree, const std::string& newComponentName, const nl::json& newComponentConfig){
        if(inManagedTree && !isManaged()){
            throw std::runtime_error("Only managed Entity can create a managed component.");
        }
        if(newComponentName.find("/")!=std::string::npos){
            std::string intermediate=EntityComponentTree<StructuredEntity,true>::resolve(newComponentName);
		    auto slash=intermediate.rfind("/");
            while(slash!=std::string::npos){
                if(hasComponent(inManagedTree,intermediate)){
                    intermediate=intermediate.substr(0,slash);
                    slash=intermediate.rfind("/");
                }else{
                    throw std::runtime_error(
                        std::string("Failed to create component at '")+
                        newComponentName+
                        std::string("' because there is no component at '")+
                        EntityComponentTree<StructuredEntity,true>::resolve(getComponentPath(inManagedTree)+"/"+intermediate)+
                        "'. Please create intermediate component first."
                    );
                }
            }
        }
        if(hasComponent(inManagedTree,newComponentName)){
            throw std::runtime_error("A component with name="+newComponentName+" already exists.");
        }
        std::shared_ptr<T> component;
        if(inManagedTree){
            component=createEntity<T>(getNewComponentConfig(inManagedTree,newComponentName,newComponentConfig));
        }else{
            component=createUnmanagedEntity<T>(getNewComponentConfig(inManagedTree,newComponentName,newComponentConfig));
        }
        auto casted=std::dynamic_pointer_cast<T>(component);
        if(!casted){
            throw std::runtime_error("The created component '"+component->getFullName()+"' is not an instance of '"+cereal::util::demangle(typeid(T).name())+"', but '"+component->getDemangledName()+"'.");
        }
        return std::move(casted);
    }

    /**
     * @brief 自身と他のEntityの間の処理順序の依存関係を設定するための関数。
     * 
     * @details addDependencyやaddDependencyCreatorを用いる。
     *          自身がmanagedな場合はSimulationManagerによって呼ばれる。
     *          エピソード開始時は全Entityの生成後、エピソード途中で生成された場合は生成されたphaseの完了後に呼ばれる。
     * 
     *          自身がunmanagedな場合、rootでなければrootによって呼ばれる。rootならばそれを管理しているインスタンスが呼び出すものとする。
     * 
     * @code
     *  
     *  void DerivedClass::setDependency(){
     *      BaseType::setDependency();
     * 
     *      // managedな場合はSimPhaseに限定
     *      addDependency(SimPhase::PERCEIVE,getComponent(true,"comp1"),getComponent(true,"comp2"));
     *      addDependency(SimPhase::BEHAVE,non_const_this(),root());
     * 
     *      // unmanagedな場合はユーザが任意の文字列でphaseを定義できる。
     *      addDependency("USER_DEFINIED_PHASE",getComponent(false,"comp3"),getComponent(false,"comp4"));
     *  }
     * 
     * @endcode
     */
    virtual void setDependency();

    /**
     * @brief phaseにおいてpredecessor -> successorの依存関係を追加する。
     * 
     * @param [in] phase        追加対象のSimPhase
     * @param [in] predecessor  先に処理されるべきStructuredEntity
     * @param [in] successor    後に処理されるべきStructuredEntity
     * 
     * @details 両方managedならSimulationManagerに追加する。
     *          少なくとも一方がunmanagedなら、同じcomponent tree上にあるものとして、
     *          そのrootが持つdependency graphに追加する。
     * 
     */
    void addDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    /**
     * @brief phaseにおいてpredecessor -> successorの依存関係を追加する。phaseをenumでなく文字列で与えるバージョン。
     * 
     */
    void addDependency(const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    /**
     * @brief phaseにおいてpredecessor -> successorの依存関係を削除する。
     * 
     * @param [in] phase        削除対象のSimPhase
     * @param [in] predecessor  先に処理されるべきだったStructuredEntity
     * @param [in] successor    後に処理されるべきだったStructuredEntity
     * 
     * @details 両方managedならSimulationManagerから削除する。
     *          少なくとも一方がunmanagedなら、同じcomponent tree上にあるものとして、
     *          そのrootが持つdependency graphから削除する。
     * 
     */
    void removeDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    /**
     * @brief phaseにおいてpredecessor -> successorの依存関係を削除する。phaseをenumでなく文字列で与えるバージョン。
     * 
     */
    void removeDependency(const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    /**
     * @brief 自身が属するcomponent treeのpathにStructuredEntityが存在するかどうかを返す。
     * 
     * @param [in] inManagedTree trueの場合はmanaged treeから、falseの場合はunmanaged treeから探す。
     * @param [in] path          検索対象のパス。先頭が"/"なら絶対パス、そうでなければ相対パスと解釈する。
     * 
     */
    bool hasComponent(bool inManagedTree,const std::string& path) const;

    /**
     * @brief 自身が属するcomponent treeのpathにあるStructuredEntityをTにcastして返す。
     * 
     * @param [in] inManagedTree trueの場合はmanaged treeから、falseの場合はunmanaged treeから探す。
     * @param [in] path          検索対象のパス。先頭が"/"なら絶対パス、そうでなければ相対パスと解釈する。
     * 
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    std::shared_ptr<T> getComponent(bool inManagedTree,const std::string& path) const{
        if(inManagedTree && !isManaged()){
            throw std::runtime_error("Only managed Entity can get a component in the managed tree.");
        }
        std::string absPath=path;
        if(absPath.size()==0 || absPath.at(0)!='/'){
            // 相対パス→絶対パスに変換
            absPath=getComponentPath(inManagedTree)+"/"+absPath;
        }
        std::shared_ptr<StructuredEntity> component;
        if(inManagedTree){
            component=getManagedComponentTree(root(inManagedTree)).get(absPath);
        }else{
            component=getUnmanagedComponentTree(root(inManagedTree)).get(absPath);
        }
        if(!component){
            throw std::runtime_error("There is no component at '"+absPath+"' in the managed tree of '"+root(inManagedTree)->getFullName()+"'.");
        }
        auto casted=std::dynamic_pointer_cast<T>(component);
        if(!casted){
            throw std::runtime_error("'"+component->getFullName()+"' is not an instance of '"+cereal::util::demangle(typeid(T).name())+"', but '"+component->getDemangledName()+"'.");
        }
        return std::move(casted);
    }

    /**
     * @brief 自身が持つmanaged componentのrangeを返す。
     * 
     * @return std::shared_ptr<T>を要素とするstd::ranges::range
     * 
     * @code
     *  // 個別にキャストする場合
     *  for(auto&& component : getManagedComponents()){
     *      if(isinstance<DerivedClass>(component)){
     *          getShared<DerivedClass>(component)->doSomething();
     *      }
     *  }
     * 
     *  // 一括でキャストする場合
     *  for(auto&& component : getManagedComponents<DerivedClass>()){
     *      if(component){ // 全要素が確実にDerivedClassだと分かっているなら省略してもよい
     *          component->doSomething();
     *      }
     *  }
     * 
     * @endcode
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    inline auto getManagedComponents() const{
        if(!isManaged()){
            throw std::runtime_error("getManagedComponents() is allowed only for managed StructuredEntity.");
        }
        return (
            getManagedComponentTree(root(true)).get_children(getComponentPath(true))
            | std::views::transform([](auto&& v){return std::dynamic_pointer_cast<T>(std::forward<decltype(v)>(v).second);})
        );
    }

    /**
     * @brief 自身が持つunmanaged componentのrangeを返す。
     * 
     * @return std::shared_ptr<T>を要素とするstd::ranges::range
     * 
     * @code
     *  // 個別にキャストする場合
     *  for(auto&& component : getUnmanagedComponents()){
     *      if(isinstance<DerivedClass>(component)){
     *          getShared<DerivedClass>(component)->doSomething();
     *      }
     *  }
     * 
     *  // 一括でキャストする場合
     *  for(auto&& component : getUnmanagedComponents<DerivedClass>()){
     *      if(component){ // 全要素が確実にDerivedClassだと分かっているなら省略してもよい
     *          component->doSomething();
     *      }
     *  }
     * 
     * @endcode
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    inline auto getUnmanagedComponents() const{
        return (
            getUnmanagedComponentTree(root(false)).get_children(getComponentPath(false))
            | std::views::transform([](auto&& v){return std::dynamic_pointer_cast<T>(std::forward<decltype(v)>(v).second);})
        );
    }

    /**
     * @brief 自身が持つunmanaged componentのphaseにおける処理順序によるソート済rangeを返す。
     * 
     * @return std::shared_ptr<T>を要素とするstd::ranges::range
     *  
     * @code
     *  for(auto&& component : getUnmanagedComponents("CONTROL")){
     *      component->control();
     *  }
     * @endcode
     * 
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    inline auto getUnmanagedComponents(const std::string& phase) const{
        auto r=root(false);
        std::size_t count=r->_updateCountOfUnmanagedComponentDependencyGraph[phase];
        if(
            !_updateCountOfUnmanagedComponentOrder.contains(phase)
            || (
                _updateCountOfUnmanagedComponentOrder[phase]
                != count
            )
        ){
            // 更新必要
            _unmanagedComponentOrder[phase].clear();
            for(auto&& instance : getUnmanagedComponents()){
                auto v=getUnmanagedComponentDependencyGraph(r,phase).get_vertex(instance);
                _unmanagedComponentOrder[phase].push_back(v);
            }
            std::ranges::sort(_unmanagedComponentOrder[phase],std::ranges::less(),
                [r,phase](auto&& v){return getUnmanagedComponentDependencyGraph(r,phase).get_index(std::forward<decltype(v)>(v));}
            );
            _updateCountOfUnmanagedComponentOrder[phase]=count;
        }
        return (
            std::ranges::subrange(_unmanagedComponentOrder[phase].begin(),_unmanagedComponentOrder[phase].end())
            | std::views::transform([r,phase](auto&& v){
                return std::dynamic_pointer_cast<T>(getUnmanagedComponentDependencyGraph(r,phase).get_instance(std::forward<decltype(v)>(v)));
            })
        );
    }


    /**
     * @brief 自身が属するunmanaged component treeのphaseにおける処理順序によるソート済rangeを返す。
     * 
     * @return std::shared_ptr<T>を要素とするstd::ranges::range
     *  
     * @code
     *  for(auto&& component : getAllUnmanagedComponentsInTree("CONTROL")){
     *      component->control();
     *  }
     * @endcode
     * 
     */
    template<std::derived_from<StructuredEntity> T=StructuredEntity>
    inline auto getAllUnmanagedComponentsInTree(const std::string& phase) const{
        return (
            getUnmanagedComponentDependencyGraph(root(false),phase).get_range()
            | std::views::transform([](auto&& v){return std::dynamic_pointer_cast<T>(v);})
        );
    }

    /**
     * @brief 自身がunmanaged componentである場合に処理順序を管理するべきphaseの一覧を返す。派生クラスで定義する。
     * 
     * @note 基本的にはSimPhaseの各値が想定されるが、それ以外の任意のphaseを管理できるようにするために文字列で保持する。
     */
    virtual std::set<std::string> getUnmanagedComponentPhases();

    protected:

    /**
     * @brief 新しいcomponentの生成用jsonを返す。派生クラスで条件付けを行えるように仮想関数としている。
     * 
     * @note ユーザによるカスタマイズは想定していない。
     * 
     * @param [in] isManaged_           生成するcomponentがmanagedかどうか。
     * @param [in] newComponentName     componentの名称。生成者のfullName+"/"+newComponentNameが新たなcomponentのfullNameとなる。
     * @param [in] newComponentConfig   componentの生成用config。createEntityを単一のjsonで呼び出すときと同じフォーマット。一部の項目は自動的に上書きされる。
     */
    virtual nl::json getNewComponentConfig(bool isManaged_, const std::string& newComponentName, const nl::json& newComponentConfig);

    /**
     * @brief 2つのmanagedなStructuredEntity間の依存関係を追加する。派生クラスで定義する。
     * 
     * @note ユーザによるカスタマイズは想定していない。
     */
    virtual void addManagedDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    /**
     * @brief 2つのmanagedなStructuredEntity間の依存関係を削除する。派生クラスで定義する。
     * 
     * @note ユーザによるカスタマイズは想定していない。
     */
    virtual void removeManagedDependency(const SimPhase& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    private:
    /**
     * @brief 他のインスタンスが持つmanaged component treeを得るためのstaticメンバ。
     */
    static EntityComponentTree<StructuredEntity,true>& getManagedComponentTree(const std::shared_ptr<StructuredEntity>& root);

    /**
     * @brief 他のインスタンスが持つunmanaged component treeを得るためのstaticメンバ。
     */
    static EntityComponentTree<StructuredEntity,false>& getUnmanagedComponentTree(const std::shared_ptr<StructuredEntity>& root);

    /**
     * @brief 他のインスタンスが持つunmanaged dependency graphを得るためのstaticメンバ。
     */
    static EntityDependencyGraph<StructuredEntity>& getUnmanagedComponentDependencyGraph(const std::shared_ptr<StructuredEntity>& root,const std::string& phase);

    /**
     * @brief rootのcomponent treeにcomponentを登録する。
     */
    static void registerComponent(bool inManagedTree,const std::shared_ptr<StructuredEntity>& root, const std::shared_ptr<StructuredEntity>& component);

    /**
     * @brief rootのdependency graphにphaseにおけるpredecessor -> successorの依存関係を登録する。
     */
    static void addUnmanagedDependency(const std::shared_ptr<StructuredEntity>& root, const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    /**
     * @brief rootのdependency graphにphaseにおけるpredecessor -> successorの依存関係を削除する。
     */
    static void removeUnmanagedDependency(const std::shared_ptr<StructuredEntity>& root, const std::string& phase,const std::shared_ptr<StructuredEntity>& predecessor, const std::shared_ptr<StructuredEntity>& successor);

    bool _isComponent; //!< 自身が他者のcomponentである(component treeのルートでない)かどうか
    std::weak_ptr<StructuredEntity> _root; //!< 自身が属するcomponent treeの最上位のEntity
    std::weak_ptr<StructuredEntity> _holder; //!< 自身が属するcomponent treeの直上のEntity
    EntityComponentTree<StructuredEntity,true> _managedComponents; //!< 自身をルートとするmanagedなcomponentのツリー
    EntityComponentTree<StructuredEntity,false> _unmanagedComponents; //!< 自身をルートとするunmanagedなcomponentのツリー
    std::map<std::string,EntityDependencyGraph<StructuredEntity>> _unmanagedComponentDependencyGraph; //!< 自身をルートとするunmanagedなcomponentの処理順管理用のグラフ
    std::map<std::string,std::size_t> _updateCountOfUnmanagedComponentDependencyGraph; //!< 自身をルートとするunmanagedなcomponentの処理順管理用のグラフの更新回数
    mutable std::map<std::string,std::vector<EntityDependencyGraph<StructuredEntity>::Vertex>> _unmanagedComponentOrder; //!< 自身の直下のunmanagedなcomponentの処理順序のキャッシュ
    mutable std::map<std::string,std::size_t> _updateCountOfUnmanagedComponentOrder; //!< 自身の直下のunmanagedなcomponentの処理順序のキャッシュが最後に更新された時点のグラフの更新回数
};

ASRC_DECLARE_DERIVED_TRAMPOLINE(StructuredEntity)
    virtual void setDependency() override{
        PYBIND11_OVERRIDE(void,Base,setDependency);
    }
    virtual std::set<std::string> getUnmanagedComponentPhases() override{
        PYBIND11_OVERRIDE(std::set<std::string>,Base,getUnmanagedComponentPhases);
    }
};


/**
 * @class PlaceHolderEntity
 * @brief component treeのうち、実体を持たないノードを表すクラス。componentのグループ化のために用いる。
 * 
 * @details FactoryのbaseNameは"PlaceHolder"とする。
 */
ASRC_DECLARE_DERIVED_CLASS_WITHOUT_TRAMPOLINE(PlaceHolderEntity,StructuredEntity)
    public:
    using BaseType::BaseType;
    static const std::string baseName; // baseNameを上書き
};

/**
 * @brief このファイルで宣言したものをPython側に公開する。Main.cppのエントリポイントから呼び出す。
 */
void exportStructuredEntity(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
