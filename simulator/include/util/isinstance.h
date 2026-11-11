/**
 * Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
 * @file
 * 
 * @brief インスタンスが属するクラスの判定に関するユーティリティ
 */
#pragma once
#include "../Common.h"
#include <memory>

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

class Entity; // forward declaration

ASRC_INLINE_NAMESPACE_BEGIN(util)

/**
 * @brief fがDerivedのインスタンスかどうかを返す。
 */
template<class Derived,class Base>
bool isinstance(const std::shared_ptr<Base>& f){
    std::shared_ptr<Derived> t=std::dynamic_pointer_cast<Derived>(f);
    return (bool)t;
}
/**
 * @brief fがDerivedのインスタンスかどうかを返す。
 */
template<class Derived,class Base>
bool isinstance(const std::shared_ptr<const Base>& f){
    std::shared_ptr<const Derived> t=std::dynamic_pointer_cast<const Derived>(f);
    return (bool)t;
}
/**
 * @brief fがDerivedのインスタンスかどうかを返す。
 */
template<class Derived,class Base>
bool isinstance(const std::weak_ptr<Base>& f){
    if(f.expired()){
        return false;
    }else{
        return isinstance<Derived>(f.lock());
    }
}
/**
 * @brief fがDerivedのインスタンスかどうかを返す。
 */
template<class Derived,class Base>
bool isinstance(const std::weak_ptr<const Base>& f){
    if(f.expired()){
        return false;
    }else{
        return isinstance<Derived>(f.lock());
    }
}

/**
 * @brief instanceがclassNameという名前のクラスのインスタンス(派生クラス含む)であるかどうかを判定する。
 * 
 * @note Factoryに登録されたEntityのサブクラスのみ判定可能。
 * 
 * @note Python側からはisinstance_nameとしてインポート、利用可能。
 */
bool isinstance(const std::shared_ptr<const Entity>& instance, const std::string& className);

/**
 * @brief instanceがbaseNameグループに属するclassNameという名前のクラスのインスタンス(派生クラス含む)であるかどうかを判定する。
 * 
 * @note Factoryに登録されたEntityのサブクラスのみ判定可能。
 * 
 * @note Python側からはisinstance_nameとしてインポート、利用可能。
 */
bool isinstance(const std::shared_ptr<const Entity>& instance,const std::string& baseName,const std::string& className);

void exportIsinstance(py::module &m);

ASRC_NAMESPACE_END(util)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
