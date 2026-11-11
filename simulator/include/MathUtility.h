/**
 * Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
 * @file
 * 
 * @brief 数学的な操作に関するユーティリティ関数を集めたファイル
 */
#pragma once
#include "Common.h"
#include <cmath>
#include <random>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/CXX11/Tensor>
#include "traits/eigen.h"

namespace py=pybind11;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

class Time;

ASRC_INLINE_NAMESPACE_BEGIN(util)

double PYBIND11_EXPORT sinc(const double &x,const double &eps=1e-3);
double PYBIND11_EXPORT sincMinusOne(const double &x,const double &eps=1e-3);
double PYBIND11_EXPORT sincMinusOne_x2(const double &x,const double &eps=1e-3);
double PYBIND11_EXPORT oneMinusCos(const double &x,const double &eps=1e-3);
double PYBIND11_EXPORT oneMinusCos_x2(const double &x,const double &eps=1e-3);
double PYBIND11_EXPORT atanx_x(const double &x,const double &eps=1e-3);
double PYBIND11_EXPORT acosOne_OnePlusx2_x(const double &x,const double &eps=1e-3);

/**
 * @brief vに直交するベクトルを一つ生成して返す。
 */
Eigen::Vector3d PYBIND11_EXPORT getOrthogonalVector(const Eigen::Vector3d &v);

/**
 * @brief 任意の3次元ベクトルxに対してMx=v×xとなる歪対称行列Mを返す。
 */
template<typename T>
Eigen::Matrix<T,3,3> skewMatrix(const Eigen::Matrix<T,3,1>& v){
    return Eigen::Matrix<T,3,3>({
        {0,-v(2),v(1)},
        {v(2),0,-v(0)},
        {-v(1),v(0),0}});
}

/**
 * @brief pを係数とするxの多項式の値を返す。
 * 
 * @param [in] p 係数のリスト(先頭が最高次係数、末尾が定数項)
 * @param [in] x 入力変数の値
 */
template<typename T, int _Rows>
T polyval(const Eigen::Matrix<T, _Rows, 1>& p,const T &x){
    Eigen::Index n=p.size();
    T xx=1.0;
    T ret=0;
    for(int i=0;i<n;++i){
        ret+=xx*p(n-1-i);
        xx*=x;
    }
    return ret;
}
/**
 * @brief pを係数とするxの多項式の導関数値を返す。
 * 
 * @param [in] p 係数のリスト(先頭が最高次係数、末尾が定数項)
 * @param [in] x 入力変数の値
 */
template<typename T, int _Rows>
Eigen::Matrix<T,_Rows,1> polyderiv(const Eigen::Matrix<T,_Rows,1>& p,const T &x){
    Eigen::Index n=p.size()-1;
    Eigen::Matrix<T,_Rows,1> ret;
    for(int i=0;i<n;++i){
        ret(i)=p(i)*(n-1-i);
    }
    return ret;
}

/**
 * @brief Tensorの次元数を返す。
 */
template<typename TensorType>
requires (
    traits::tensor<TensorType> || traits::matrix<TensorType>
)
auto getTensorDimensions(const TensorType& t){
    if constexpr (traits::matrix<TensorType>){
        if constexpr  (TensorType::IsVectorAtCompileTime){
            return Eigen::DSizes<Eigen::Index,1>{t.size()};
        }else{
            return Eigen::DSizes<Eigen::Index,2>{t.rows(),t.cols()};
        }
    }else if constexpr (traits::has_dimensions<TensorType>){
        return t.dimensions();
    }else if constexpr (traits::has_sizes<TensorType>){
        return t.sizes();
    }else{
        return Eigen::TensorEvaluator<const TensorType,Eigen::DefaultDevice>(t,Eigen::DefaultDevice()).dimensions();
    }
}

/**
 * @brief Tensorの要素数を返す。
 */
template<typename TensorType>
requires (
    traits::tensor<TensorType> || traits::matrix<TensorType>
)
auto getTensorSize(const TensorType& t){
    if constexpr (traits::has_size<TensorType>){
        return t.size();
    }else{
        auto dimensions=getTensorDimensions(t);
        typename Eigen::internal::traits<TensorType>::Index ret=1;
        for(auto d : dimensions){
            ret*=d;
        }
        return ret;
    }
}

/**
 * @brief 引数を、指定したLayoutを持つテンソルに変更する。
 * 
 * @tparam ReturnLayout Eigen::RowMajorまたはEigen::ColMajor
 * @param [in] v 変更元のテンソル
 * 
 * @returns 指定したLayoutを持つテンソル。
 * 
 * @details
 *      - 引数がTensor(TensorBase)のとき
 *          - 引数のLayoutがReturnLayoutと異なる、又は引数が右辺値の場合は、Tensorで値として返す。
 *          - 引数のLayoutがReturnLayoutと同じで、かつ引数が左辺値の場合は、std::forwardで引数の参照をそのまま返す。
 *      - 引数がMatrix(DenseBase)のとき
 *          以下を全て満たす場合は元データへのTensorMap/const TensorMapで返し、そうでなければTensorで返す。
 *          - 引数がraw access可能
 *          - 引数のLayoutとReturnLayoutが同じ、又はコンパイル時Vector(RawType::IsVectorAtCompileTimeがtrue)である
 *          - 引数が右辺値ではない
 *      - 引数がそれ以外(スカラー)のとき
 *          引数が右辺値でなければ元データへのTensorMap/const TensorMapで返し、右辺値ならばTensorで返す。
 *          
 */
template<int ReturnLayout,typename ValueType>
decltype(auto) toTensor(ValueType&& v){
    using RawType = std::remove_cv_t<std::remove_reference_t<ValueType>>;
    using Scalar=traits::Scalar<RawType>;
    constexpr int Layout = traits::Layout<RawType>;
    constexpr auto NumDimensions=traits::NumDimensions<RawType>;
    using ReturnTensorType = Eigen::Tensor<Scalar,NumDimensions,ReturnLayout>;
    using ReturnMapType = std::conditional_t<
        std::is_const_v<std::remove_reference_t<ValueType>>,
        const Eigen::TensorMap<const ReturnTensorType>,
        Eigen::TensorMap<ReturnTensorType>
    >;
    if constexpr (traits::tensor<RawType>){
        if constexpr (ReturnLayout == Layout){
            if constexpr(std::is_rvalue_reference_v<ValueType&&>){
                return RawType(std::forward<ValueType>(v));
            }else{
                return std::forward<ValueType>(v);
            }
        }else{
            Eigen::DSizes<Eigen::Index, NumDimensions> shuffle;
            for(Eigen::Index i=0; i<NumDimensions; ++i){
                shuffle[i]=NumDimensions-1-i;
            }
            return Eigen::Tensor<Scalar,NumDimensions,ReturnLayout>(std::forward<ValueType>(v).swap_layout().shuffle(shuffle));
        }
    }else if constexpr (traits::matrix<RawType>){
        if constexpr (
            traits::isRawAccessible<RawType>
            && (ReturnLayout == Layout || NumDimensions<=1)
            && !std::is_rvalue_reference_v<ValueType&&>
        ){
            if constexpr (NumDimensions==1){
                return ReturnMapType(std::forward<ValueType>(v).data(), v.size());
            }else{
                return ReturnMapType(std::forward<ValueType>(v).data(), v.rows(), v.cols());
            }
        }else{
            if constexpr (NumDimensions==1){
                ReturnTensorType ret(v.size());
                Eigen::Map<typename RawType::PlainMatrix> map(ret.data(),v.rows(),v.cols());
                map=std::forward<ValueType>(v);
                return ret;
            }else{
                ReturnTensorType ret(v.rows(),v.cols());
                if constexpr (ReturnLayout == Layout){
                    Eigen::Map<typename RawType::PlainMatrix> map(ret.data(),v.rows(),v.cols());
                    map=std::forward<ValueType>(v);
                    return ret;
                }else{
                    Eigen::Map<typename RawType::PlainMatrix> map(ret.data(),v.cols(),v.rows());
                    map=std::forward<ValueType>(v).transpose();
                    return ret;
                }
            }
        }
    }else{ //Scalar
        if constexpr(std::is_rvalue_reference_v<ValueType&&>){
            ReturnTensorType ret;
            ret()=std::forward<ValueType>(v);
            return ret;
        }else{
            return ReturnMapType(&std::forward<ValueType>(v));
        }
    }
}

template<typename ValueType>
decltype(auto) toTensor(ValueType&& v){
    return toTensor<traits::Layout<ValueType>>(std::forward<ValueType>(v));
}

/**
 * @brief TensorをそのNumDimensionsの値に応じて対応するMatrix,Vector又はスカラーに変換する。
 * 
 * @tparam ReturnLayout Eigen::RowMajorまたはEigen::ColMajor。省略時は引数のLayoutと同じ。
 * @param [in] t        変換元のTensor。Eigen::TensorBaseから派生したものを受け取れるので、式のまま与えてよい。
 * 
 * @details
 *      以下を全て満たす場合は元データへのMap/const Mapで返し、そうでなければMatrixで返す。
 *      - 引数がraw access可能
 *      - 引数のLayoutとReturnLayoutが同じ
 *      - 引数が右辺値ではない
 * 
 *      NumDimensionsが1のときはコンパイル時Vector(ColMajorならRows=-1,Cols=1で、RowMajorならRows=1,Cols=-1)として返す。
 *      NumDimensionsが2のときはMatrixXとして返す。
 *      なお、NumDimensionsが0のときはスカラー又はスカラー参照として返すものとする。
 */
template<int ReturnLayout,typename TensorType,
    typename RawType = std::remove_cv_t<std::remove_reference_t<TensorType>>
>
requires (
    traits::tensor<RawType>
    && traits::NumDimensions<RawType> <= 2
)
decltype(auto) tensorToMatrix(TensorType&& t){
    using Scalar=traits::Scalar<RawType>;
    constexpr auto NumDimensions = traits::NumDimensions<RawType>;
    constexpr int Layout = traits::Layout<RawType>;
    using ReturnMatrixType = std::conditional_t<
        NumDimensions==0,
        Scalar,
        std::conditional_t<
            NumDimensions==2,
            Eigen::Matrix<Scalar,-1,-1,ReturnLayout>,
            std::conditional_t<
                (ReturnLayout & Eigen::RowMajor)==Eigen::RowMajor,
                Eigen::Matrix<Scalar,1,-1,Eigen::RowMajor>,
                Eigen::Matrix<Scalar,-1,1,Eigen::ColMajor>
            >
        >
    >;
    using ReturnMapType = std::conditional_t<
        NumDimensions==0,
        std::conditional_t<
            std::is_const_v<std::remove_reference_t<TensorType>>,
            const Scalar&,
            Scalar&
        >,
        std::conditional_t<
            std::is_const_v<std::remove_reference_t<TensorType>>,
            const Eigen::Map<const ReturnMatrixType>,
            Eigen::Map<ReturnMatrixType>
        >
    >;
    if constexpr (
        traits::isRawAccessible<RawType>
        && (ReturnLayout == Layout || NumDimensions <= 1)
        && !std::is_rvalue_reference_v<TensorType&&>
    ){
        if constexpr (NumDimensions == 0){
            return ReturnMapType(std::forward<TensorType>(t)());
        }else if constexpr (NumDimensions == 1){
            Eigen::Index size=getTensorSize(std::forward<TensorType>(t));
            return ReturnMapType(std::forward<TensorType>(t).data(), size);
        }else{
            auto dimensions=getTensorDimensions(std::forward<TensorType>(t));
            return ReturnMapType(std::forward<TensorType>(t).data(), dimensions[0], dimensions[1]);
        }
    }else{
        using BaseTensorType=Eigen::Tensor<Scalar,NumDimensions,Layout>;
        if constexpr (NumDimensions == 0){
            BaseTensorType ret;
            ret=std::forward<TensorType>(t);
            return ReturnMatrixType(ret());
        }else if constexpr (NumDimensions == 1){
            Eigen::Index size=getTensorSize(std::forward<TensorType>(t));
            ReturnMatrixType ret(size);
            Eigen::TensorMap<BaseTensorType> map(ret.data(),size);
            map=std::forward<TensorType>(t);
            return ret;
        }else{
            auto dimensions=getTensorDimensions(std::forward<TensorType>(t));
            ReturnMatrixType ret(dimensions[0], dimensions[1]);
            if constexpr (ReturnLayout == Layout){
                Eigen::TensorMap<BaseTensorType> map(ret.data(),dimensions[0],dimensions[1]);
                map=std::forward<TensorType>(t);
                return ret;
            }else{
                Eigen::DSizes<Eigen::Index, NumDimensions> shuffle;
                for(Eigen::Index i=0; i<NumDimensions; ++i){
                    shuffle[i]=NumDimensions-1-i;
                }
                Eigen::TensorMap<BaseTensorType> map(ret.data(),dimensions[1],dimensions[0]);
                map=std::forward<TensorType>(t).shuffle(shuffle);
                return ret;
            }
        }
    }
}
template<typename TensorType,
    typename RawType = std::remove_cv_t<std::remove_reference_t<TensorType>>
>
requires (
    traits::tensor<RawType>
    && traits::NumDimensions<RawType> <= 2
)
decltype(auto) tensorToMatrix(TensorType&& t){
    constexpr int Layout = traits::Layout<RawType>;
    return tensorToMatrix<Layout>(std::forward<TensorType>(t));
}

/**
 * @brief Eigen::TensorBaseのLayoutを指定したものに変更し、0次元はスカラーに、1次元はEigen::Vectorに、2次元はEigen::Matrixに、3次元以上はEigen::Tensorに変換する。
 * 
 * @tparam ReturnLayout Eigen::RowMajorまたはEigen::ColMajor
 * @param [in] v 変更元のテンソル
 * 
 * @returns LayoutがReturnLayoutに変更され、次元に応じた型に変換されたもの。参照(又はMap)で返せる場合は参照(又はMap)で返す。
 * 
 * @details
 *      以下の条件を満たす場合は元データへの参照で返す。
 *      - NumDimension<=2のときは、以下を全て満たす場合は元データへの参照(Eigen::Map又はScalar&)で返し、そうでなければ値(Eigen::Matrix又はScalar)として返す。
 *          - 引数がraw access可能
 *          - 引数のLayoutとReturnLayoutが同じか、NumDimensions<=1でLayoutに依存しない
 *          - 引数が右辺値ではない
 *      - NumDimensions>=3のときは、引数のLayoutとReturnLayoutが同じならば引数をforwardでそのまま返し、異なる場合はEigen::Tensorとして返す。
 */
template<int ReturnLayout,typename TensorType>
decltype(auto) adjustTensorLayoutAndType(TensorType&& v){
    using RawType = std::remove_cv_t<std::remove_reference_t<TensorType>>;

    if constexpr (traits::NumDimensions<RawType> <= 2){
        //スカラー値orベクトル値or行列値
        return tensorToMatrix<ReturnLayout>(std::forward<TensorType>(v));
    }else{
        //3次元以上のテンソルはテンソルのまま。
        return toTensor<ReturnLayout>(std::forward<TensorType>(v));
    }
}

/**
 * @brief テンソルの最外次元の指定インデックスで切り取ったchipをTensorMapとして返す。
 * 
 * @param [in] t     対象のテンソル
 * @param [in] index 切り取りたいインデックス
 * 
 * @details t.chip<0>(index)又はt.chip<NumDimensions-1>(index)に相当するが、TensorChippingOpは最外次元であってもraw accessが不可能なため別途実装するもの。
 */
template<traits::tensor TensorType>
requires (
    traits::NumDimensions<TensorType> >= 1
    && traits::isRawAccessible<TensorType>
)
auto outerChip(TensorType& t, const Eigen::Index& index){
    constexpr auto NumDimensions = traits::NumDimensions<TensorType>;
    using MapType=Eigen::TensorMap<Eigen::Tensor<traits::Scalar<TensorType>, NumDimensions-1, traits::Layout<TensorType>>>;
    if constexpr (NumDimensions == 1){
        return MapType(t.data()+index);
    }else{
        constexpr Eigen::Index outerDimIndex = (traits::isRowMajor<TensorType> ? 0 : NumDimensions-1);
        constexpr Eigen::Index dimIndexOffset = (traits::isRowMajor<TensorType> ? 1 : 0);
        Eigen::Index stride=t.size()/t.dimension(outerDimIndex);
        Eigen::DSizes<Eigen::Index, NumDimensions-1> subDimensions;
        for(Eigen::Index i=0; i<NumDimensions-1; ++i){
            subDimensions[i]=t.dimension(i+dimIndexOffset);
        }
        return MapType(t.data()+stride*index,subDimensions);
    }
}

/**
 * @brief テンソルの最外次元の指定インデックスで切り取ったchipをTensorMapとして返す。
 * 
 * @param [in] t     対象のテンソル
 * @param [in] index 切り取りたいインデックス
 * 
 * @details t.chip<0>(index)又はt.chip<NumDimensions-1>(index)に相当するが、TensorChippingOpは最外次元であってもraw accessが不可能なため別途実装するもの。
 */
template<traits::tensor TensorType>
requires (
    traits::NumDimensions<TensorType> >= 1
    && traits::isRawAccessible<TensorType>
)
auto outerChip(const TensorType& t, const Eigen::Index& index){
    constexpr auto NumDimensions = traits::NumDimensions<TensorType>;
    using MapType=Eigen::TensorMap<const Eigen::Tensor<traits::Scalar<TensorType>, NumDimensions-1, traits::Layout<TensorType>>>;
    if constexpr (NumDimensions == 1){
        return MapType(t.data()+index);
    }else{
        constexpr Eigen::Index outerDimIndex = (traits::isRowMajor<TensorType> ? 0 : NumDimensions-1);
        constexpr Eigen::Index dimIndexOffset = (traits::isRowMajor<TensorType> ? 1 : 0);
        Eigen::Index stride=t.size()/t.dimension(outerDimIndex);
        Eigen::DSizes<Eigen::Index, NumDimensions-1> subDimensions;
        for(Eigen::Index i=0; i<NumDimensions-1; ++i){
            subDimensions[i]=t.dimension(i+dimIndexOffset);
        }
        return MapType(t.data()+stride*index,subDimensions);
    }
}

/**
 * @brief 1次元テーブルデータを内挿する際に参照すべきインデックスと重み付け係数を返す。
 * 
 * @param [in] x     テーブルのデータ点のリスト(ソート済)
 * @param [in] p     内挿したい点の位置
 * @param [in] clamp trueにすると、p が x の範囲外にある場合、xの端点にclampされる。falseの場合、線形外挿となる。
 * 
 * @returns \f$ p = x(i) + a (x(i+1)-x(i)) \f$ を満たす\f$(i,a)\f$
 */
template<typename VectorType>
requires (
    ( traits::matrix<VectorType> )
    || ( traits::tensor<VectorType>)
)
std::pair<Eigen::Index,traits::Scalar<VectorType>> _search1d(const VectorType &x,const traits::Scalar<VectorType> &p, bool clamp=false){
    Eigen::Index lb=0;
    Eigen::Index ub;
    if constexpr (traits::matrix<VectorType>){
        assert(x.rows()==1 || x.cols()==1);
        ub=x.size()-2;
    }else if constexpr (traits::tensor<VectorType>){
        if constexpr (traits::NumDimensions<VectorType> > 1){
            auto dims=getTensorDimensions(x);
            bool found_more_than_one=false;
            for(auto d: dims){
                if(d>1){
                    if(found_more_than_one){
                        throw std::runtime_error("asrc::core::_search1d failed. x is not 1-dim tensor.");
                    }else{
                        found_more_than_one=true;
                    }
                }
            }
            return _search1d(x.reshape(Eigen::DSizes<Eigen::Index,1>{getTensorSize(x)}),p,clamp);
        }
        ub=getTensorSize(x)-2;
    }else{
        ub=x.size()-2;
    }
    if(ub<0){
        // x has only one element
        return {lb,x(lb)};
    }
    if(p<x(lb)){
        if(clamp){
            return {lb,0};
        }else{
            return {lb,(p-x(lb))/(x(lb+1)-x(lb))};
        }
    }
    if(x(ub)<=p){
        if(x(ub+1)<=p && clamp){
            return {ub,1};
        }else{
            return {ub,(p-x(ub))/(x(ub+1)-x(ub))};
        }
    }
    Eigen::Index i;
    while(ub-lb>1){
        i=(lb+ub)/2;
        if(x(i)<=p){
            lb=i;
        }else{// if(p<x(i)){
            ub=i;
        }
    }
    return {lb,(p-x(lb))/(x(lb+1)-x(lb))};
}

template<std::size_t NumTableDimensions,typename TablePointType,typename PointType,
    typename Scalar=traits::Scalar<TablePointType>
>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> == 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<PointType>,Scalar>
)
std::tuple<Eigen::VectorX<Eigen::Index>,Eigen::VectorX<Scalar>,Eigen::VectorX<Scalar>,Eigen::VectorX<bool>> _searchnd(
    const std::array<TablePointType,NumTableDimensions> point,
    const PointType& x,
    bool clamp=false
){
    Eigen::VectorX<Eigen::Index> indices(NumTableDimensions);
    Eigen::VectorX<Scalar> ratios(NumTableDimensions);
    Eigen::VectorX<Scalar> deltas(NumTableDimensions);
    Eigen::VectorX<bool> clamped(NumTableDimensions);
    Scalar xx;
    for(Eigen::Index d=0;d<NumTableDimensions;++d){
        auto& p=point[d];
        if constexpr (traits::NumDimensions<PointType> == 0){
            if constexpr (traits::tensor<PointType>){
                xx=x();
            }else{
                xx=x;
            }
        }else{
            xx=x(d);
        }
        auto [index,ratio]=_search1d(p,xx,clamp);
        indices(d) = index;
        ratios(d) = ratio;
        deltas(d) = p(index+1)-p(index);
        clamped(d) = clamp && (xx<p(0) || xx>=p(getTensorSize(p)-1));
    }
    return {
        std::move(indices),
        std::move(ratios),
        std::move(deltas),
        std::move(clamped)
    };
}

/**
 * @internal
 * @brief [For internal use] テーブルデータの線形内挿処理を実際に行う関数。
 * 
 */
template<std::size_t NumTableDimensions, traits::tensor TableValueType, traits::matrix IndexType, traits::matrix RatioType>
auto _interp(const TableValueType& table, const IndexType& indices, const RatioType& ratios){
    constexpr std::size_t NumValueDimensions=traits::NumDimensions<TableValueType>-NumTableDimensions;
    using ReturnValueType=Eigen::Tensor<traits::Scalar<TableValueType>,NumValueDimensions,traits::Layout<TableValueType>>;
    if constexpr(NumTableDimensions==1){
        auto upper=outerChip(table,indices(0,0)+1);
        auto lower=outerChip(table,indices(0,0));
        return ReturnValueType(ratios(0,0)*(upper-lower)+lower);
    }else{
        auto nextIndices=indices.block(1,0,indices.rows()-1,1);
        auto nextRatios=ratios.block(1,0,ratios.rows()-1,1);
        auto upper=_interp<NumTableDimensions-1>(outerChip(table,indices(0,0)+1),nextIndices,nextRatios);
        auto lower=_interp<NumTableDimensions-1>(outerChip(table,indices(0,0)),nextIndices,nextRatios);
        return ReturnValueType(ratios(0,0)*(upper-lower)+lower);
    }
}

/**
 * @internal
 * @brief [For internal use] テーブルデータを線形内挿した際の偏導関数値の計算を実際に行う関数。
 * 
 */
template<std::size_t NumTableDimensions,traits::tensor TableValueType, traits::matrix IndexType, traits::matrix RatioType, traits::matrix DeltaType>
auto _grad(std::size_t gradDimensionIndex,const TableValueType& table, const IndexType& indices, const RatioType& ratios, const DeltaType& deltas, bool clamped=false){
    constexpr std::size_t NumValueDimensions=traits::NumDimensions<TableValueType>-NumTableDimensions;
    using ReturnValueType=Eigen::Tensor<traits::Scalar<TableValueType>,NumValueDimensions,traits::Layout<TableValueType>>;
    if(clamped){
        //範囲外でclampされた点の微分は0とみなす。
        auto totalDimensions=getTensorDimensions(table);
        Eigen::DSizes<Eigen::Index, NumValueDimensions> returnDimensions;
        for(Eigen::Index i=0; i<NumValueDimensions; ++i){
            returnDimensions[i]=totalDimensions[NumTableDimensions+i];
        }
        return ReturnValueType(returnDimensions).setZero();
    }
    if constexpr(NumTableDimensions==1){
        auto upper=outerChip(table,indices(0,0)+1);
        auto lower=outerChip(table,indices(0,0));
        if(gradDimensionIndex==0){
            return ReturnValueType{(upper-lower)/deltas(0,0)};
        }else{
            return ReturnValueType{ratios(0,0)*(upper-lower)+lower};
        }
    }else{
        auto nextIndices=indices.block(1,0,indices.rows()-1,1);
        auto nextRatios=ratios.block(1,0,ratios.rows()-1,1);
        auto nextDeltas=deltas.block(1,0,deltas.rows()-1,1);
        auto upper=_grad<NumTableDimensions-1>(gradDimensionIndex-1,outerChip(table,indices(0,0)+1),nextIndices,nextRatios,nextDeltas);
        auto lower=_grad<NumTableDimensions-1>(gradDimensionIndex-1,outerChip(table,indices(0,0)),nextIndices,nextRatios,nextDeltas);
        if(gradDimensionIndex==0){
            return ReturnValueType{(upper-lower)/deltas(0,0)};
        }else{
            return ReturnValueType{ratios(0,0)*(upper-lower)+lower};
        }
    }
}

/**
 * @brief 各要素がV次元のテンソル値であるN次元テーブルデータを複数の点xにおいて線形内挿する。
 * 
 * @param [in] point テーブルの各次元のデータ点のソート済リスト
 * @param [in] table テーブルの値リスト(N+V次元のテンソルで、前のN次元がテーブルの次元、後のV次元が各要素の値を表す)
 * @param [in] x     内挿したい点の位置(K+1次元のテンソルで、末尾の次元が各内挿点を表すN次元ベクトル)
 * @param [in] clamp trueにすると、x が point の範囲外にある場合、 point の端点にclampされる。falseの場合、線形外挿となる。
 * 
 * @returns 内挿された値を表すK+V次元テンソル。ただし0次元はスカラー、1次元はEigen::VectorX、2次元はEigen::MatrixXとして返される。
 * 
 * @details
 *          V>=0, N>=1, K>=0であること。
 *          N=1かつK=0の場合は、xとしてスカラーを与えても良い。 
 */
template<typename TableValueType,typename TablePointType,typename PointType,std::size_t NumTableDimensions>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TableValueType> >= NumTableDimensions
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> >= 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<TablePointType>,traits::Scalar<TableValueType>>
    && std::convertible_to<traits::Scalar<PointType>,traits::Scalar<TableValueType>>
)
auto interpn_impl(const std::array<TablePointType,NumTableDimensions>& point,const TableValueType &table,const PointType &x, bool clamp=false){
    if constexpr (
        (traits::tensor<TableValueType> && traits::isRowMajor<TableValueType>)
        && (traits::tensor<PointType> && traits::isRowMajor<PointType>)
    ){
        constexpr auto NumPointDimensions=traits::NumDimensions<PointType>;
        if constexpr (traits::NumDimensions<PointType> <= 1){
            //1点
            auto [indices,ratios,deltas,clamped]=_searchnd(point,x,clamp);
            return _interp<NumTableDimensions>(table,indices,ratios);
        }else{
            //複数点
            constexpr auto NumValueDimensions=traits::NumDimensions<TableValueType>-NumTableDimensions;
            constexpr auto NumReturnDimensions=NumValueDimensions+(NumPointDimensions-1);
            auto totalDimensions=getTensorDimensions(table);
            auto pointDimensions=getTensorDimensions(x);
            Eigen::DSizes<Eigen::Index, NumReturnDimensions> returnDimensions;
            for(Eigen::Index i=0; i<NumPointDimensions-1; ++i){
                returnDimensions[i]=pointDimensions[i];
            }
            for(Eigen::Index i=0; i<NumValueDimensions; ++i){
                returnDimensions[NumPointDimensions-1+i]=totalDimensions[NumTableDimensions+i];
            }
            Eigen::Tensor<traits::Scalar<TableValueType>,NumReturnDimensions,Eigen::RowMajor> ret(returnDimensions);

            for(Eigen::Index p=0;p<pointDimensions[0];++p){
                outerChip(ret,p)=interpn_impl(point,table,outerChip(x,p),clamp);
            }
            return ret;
        }
    }else{
        return interpn_impl(point,toTensor<Eigen::RowMajor>(table),toTensor<Eigen::RowMajor>(x),clamp);
    }
}

/**
 * @brief 各要素がV次元のテンソル値であるN次元テーブルデータを複数の点xにおいて線形内挿する。
 * 
 * @tparam ReturnLayout 返り値のLeyout。Eigen::RowMajorまたはEigen::ColMajor。省略時はEigan::ColMajor。
 * @param [in] point テーブルの各次元のデータ点のソート済リスト
 * @param [in] table テーブルの値リスト(N+V次元のテンソルで、前のN次元がテーブルの次元、後のV次元が各要素の値を表す)
 * @param [in] x     内挿したい点の位置(K+1次元のテンソルで、末尾の次元が各内挿点を表すN次元ベクトル)
 * @param [in] clamp trueにすると、x が point の範囲外にある場合、 point の端点にclampされる。falseの場合、線形外挿となる。
 * 
 * @returns 内挿された値を表すK+V次元テンソル。ただし0次元はスカラー、1次元はEigen::VectorX、2次元はEigen::MatrixXとして返される。
 * 
 * @details
 *          V>=0, N>=1, K>=0であること。
 *          N=1かつK=0の場合は、xとしてスカラーを与えても良い。 
 */
template<int ReturnLayout,typename TableValueType,typename TablePointType,typename PointType,std::size_t NumTableDimensions>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TableValueType> >= NumTableDimensions
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> >= 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<TablePointType>,traits::Scalar<TableValueType>>
    && std::convertible_to<traits::Scalar<PointType>,traits::Scalar<TableValueType>>
)
auto interpn(const std::array<TablePointType,NumTableDimensions>& point,const TableValueType &table,const PointType &x, bool clamp=false){
    return adjustTensorLayoutAndType<ReturnLayout>(interpn_impl(point,table,x,clamp));
}
template<typename TableValueType,typename TablePointType,typename PointType,std::size_t NumTableDimensions>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TableValueType> >= NumTableDimensions
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> >= 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<TablePointType>,traits::Scalar<TableValueType>>
    && std::convertible_to<traits::Scalar<PointType>,traits::Scalar<TableValueType>>
)
auto interpn(const std::array<TablePointType,NumTableDimensions>& point,const TableValueType &table,const PointType &x, bool clamp=false){
    return interpn<Eigen::ColMajor>(point,table,x,clamp);
}

/**
 * @brief 各要素がV次元のテンソル値であるN次元テーブルデータを複数の点xにおいて線形内挿した際の偏導関数値を返す。
 * 
 * @param [in] point テーブルの各次元のデータ点のソート済リスト
 * @param [in] table テーブルの値リスト(N+V次元のテンソルで、前のN次元がテーブルの次元、後のV次元が各要素の値を表す)
 * @param [in] x     内挿したい点の位置(K+1次元のテンソルで、末尾の次元が各内挿点を表すN次元ベクトル)
 * @param [in] clamp trueにすると、x が point の範囲外にある場合、 point の端点にclampされ、その次元の偏導関数値は0として扱う。falseの場合、線形外挿として扱う。
 * 
 * @returns 各点の偏導関数値を表すK+1+V次元テンソル。ただし0次元はスカラー、1次元はEigen::VectorX、2次元はEigen::MatrixXとして返される。
 * 
 * @details
 *          返り値の1〜K次元目は、xの各点に対応する。
 *          K+1次元目は、どの次元で偏微分するかに対応する。
 *          K+2〜K+2+V次元目は、V次元テンソルの各要素をi次元目の変数で偏微分した値(これもV次元テンソル)が入る
 * 
 *          V>=0, N>=1, K>=0であること。
 *          N=1かつK=0の場合は、xとしてスカラーを与えても良い。 
 */
template<typename TableValueType,typename TablePointType,typename PointType,std::size_t NumTableDimensions>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TableValueType> >= NumTableDimensions
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> >= 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<TablePointType>,traits::Scalar<TableValueType>>
    && std::convertible_to<traits::Scalar<PointType>,traits::Scalar<TableValueType>>
)
auto interpgradn_impl(const std::array<TablePointType,NumTableDimensions>& point,const TableValueType &table,const PointType &x, bool clamp=false){
    if constexpr (
        (traits::tensor<TableValueType> && traits::isRowMajor<TableValueType>)
        && (traits::tensor<PointType> && traits::isRowMajor<PointType>)
    ){
        constexpr auto NumPointDimensions=traits::NumDimensions<PointType>;
        if constexpr (NumPointDimensions==0){
            //xがスカラーの場合(1点かつ1次元テーブル)
            auto [indices,ratios,deltas,clamped]=_searchnd(point,x,clamp);
            return _grad<NumTableDimensions>(0,table,indices,ratios,deltas,clamped(0));
        }else{
            constexpr auto NumValueDimensions=traits::NumDimensions<TableValueType>-NumTableDimensions;
            constexpr auto NumReturnDimensions=1+NumValueDimensions+(NumPointDimensions-1);
            auto totalDimensions=getTensorDimensions(table);
            auto pointDimensions=getTensorDimensions(x);
            Eigen::DSizes<Eigen::Index, NumReturnDimensions> returnDimensions;
            for(Eigen::Index i=0; i<NumPointDimensions-1; ++i){
                returnDimensions[i]=pointDimensions[i];
            }
            returnDimensions[NumPointDimensions-1]=NumTableDimensions;
            for(Eigen::Index i=0; i<NumValueDimensions; ++i){
                returnDimensions[NumPointDimensions+i]=totalDimensions[NumTableDimensions+i];
            }

            Eigen::Tensor<traits::Scalar<TableValueType>,NumReturnDimensions,Eigen::RowMajor> ret(returnDimensions);

            if constexpr (NumPointDimensions == 1){
                //1点
                auto [indices,ratios,deltas,clamped]=_searchnd(point,x,clamp);
                for(Eigen::Index d=0;d<NumTableDimensions;++d){
                    outerChip(ret,d)=_grad<NumTableDimensions>(d,table,indices,ratios,deltas,clamped(d));
                }
            }else{
                //複数点
                for(Eigen::Index p=0;p<pointDimensions[0];++p){
                    outerChip(ret,p)=interpgradn_impl(point,table,outerChip(x,p),clamp);
                }
            }
            return ret;
        }
    }else{
        return interpgradn_impl(point,toTensor<Eigen::RowMajor>(table),toTensor<Eigen::RowMajor>(x),clamp);
    }
}

/**
 * @brief 各要素がV次元のテンソル値であるN次元テーブルデータを複数の点xにおいて線形内挿した際の偏導関数値を返す。
 * 
 * @tparam ReturnLayout 返り値のLeyout。Eigen::RowMajorまたはEigen::ColMajor。省略時はEigan::ColMajor。
 * @param [in] point テーブルの各次元のデータ点のソート済リスト
 * @param [in] table テーブルの値リスト(N+V次元のテンソルで、前のN次元がテーブルの次元、後のV次元が各要素の値を表す)
 * @param [in] x     内挿したい点の位置(K+1次元のテンソルで、末尾の次元が各内挿点を表すN次元ベクトル)
 * @param [in] clamp trueにすると、x が point の範囲外にある場合、 point の端点にclampされ、その次元の偏導関数値は0として扱う。falseの場合、線形外挿として扱う。
 * 
 * @returns 各点の偏導関数値を表すK+1+V次元テンソル。ただし0次元はスカラー、1次元はEigen::VectorX、2次元はEigen::MatrixXとして返される。
 * 
 * @details
 *          返り値の1〜K次元目は、xの各点に対応する。
 *          K+1次元目は、どの次元で偏微分するかに対応する。
 *          K+2〜K+2+V次元目は、V次元テンソルの各要素をi次元目の変数で偏微分した値(これもV次元テンソル)が入る
 * 
 *          V>=0, N>=1, K>=0であること。
 *          N=1かつK=0の場合は、xとしてスカラーを与えても良い。 
 */
template<int ReturnLayout,typename TableValueType,typename TablePointType,typename PointType,std::size_t NumTableDimensions>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TableValueType> >= NumTableDimensions
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> >= 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<TablePointType>,traits::Scalar<TableValueType>>
    && std::convertible_to<traits::Scalar<PointType>,traits::Scalar<TableValueType>>
)
auto interpgradn(const std::array<TablePointType,NumTableDimensions>& point,const TableValueType &table,const PointType &x, bool clamp=false){
    return adjustTensorLayoutAndType<ReturnLayout>(interpgradn_impl(point,table,x,clamp));
}

template<typename TableValueType,typename TablePointType,typename PointType,std::size_t NumTableDimensions>
requires (
    NumTableDimensions >= 1
    && traits::NumDimensions<TableValueType> >= NumTableDimensions
    && traits::NumDimensions<TablePointType> >= 1
    && (
        traits::NumDimensions<PointType> >= 1
        || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
    )
    && std::convertible_to<traits::Scalar<TablePointType>,traits::Scalar<TableValueType>>
    && std::convertible_to<traits::Scalar<PointType>,traits::Scalar<TableValueType>>
)
auto interpgradn(const std::array<TablePointType,NumTableDimensions>& point,const TableValueType &table,const PointType &x, bool clamp=false){
    return interpgradn<Eigen::ColMajor>(point,table,x,clamp);
}

/**
 * @brief nloptの最適化対象にstd::functionを与えるためのdelegator
 * 
 * @code
 *  // 評価関数をラムダ式等で実装する。
 *  auto obj=[&](unsigned int n,const double* x,double* grad,void* data){
 *      ...
 *      return value_objective_function;
 *  }
 * 
 *  // nlopt_delegatorでラップする
 *  nlopt_delegator obj_d(&obj,nullptr); // 第2引数は上記のラムダに渡すdata
 *  nlopt::opt opt=nlopt::opt(nlopt::algorithm::LN_BOBYQA,6); //アルゴリズムと変数の次元を指定する。
 *  
 *  // nloptの目的関数にはnlopt_delegateeの関数ポインタと、それに渡すdataとしてnlopt_delegatorのポインタを渡す。
 *  opt.set_min_objective(nlopt_delegatee,&obj_d);
 * 
 * @endcode
 */
struct PYBIND11_EXPORT nlopt_delegator{
    std::function<double(unsigned int,const double*,double*,void*)>* func;
    void* data;
    nlopt_delegator(std::function<double(unsigned int,const double*,double*,void*)>* func_,void* data_);
};
/**
 * @brief nloptに最適化対象として渡す関数
 * 
 * @code
 *  // 評価関数をラムダ式等で実装する。
 *  auto obj=[&](unsigned int n,const double* x,double* grad,void* data){
 *      ...
 *      return value_objective_function;
 *  }
 * 
 *  // nlopt_delegatorでラップする
 *  nlopt_delegator obj_d(&obj,nullptr); // 第2引数は上記のラムダに渡すdata
 *  nlopt::opt opt=nlopt::opt(nlopt::algorithm::LN_BOBYQA,6); //アルゴリズムと変数の次元を指定する。
 *  
 *  // nloptの目的関数にはnlopt_delegateeの関数ポインタと、それに渡すdataとしてnlopt_delegatorのポインタを渡す。
 *  opt.set_min_objective(nlopt_delegatee,&obj_d);
 * 
 * @endcode
 */
double PYBIND11_EXPORT nlopt_delegatee(unsigned int n,const double* x,double* grad,void* data);

/**
 * @brief 有本・ポッターの解法でLQRの最適ゲインを求める。
 * 
 * @details 状態方程式が\f$ dz/dt=Az+Bv \f$、評価関数が\f$ J=\int (z^T Q z + v^T R v) dt \f$ である系に対し、
 *      最適な制御入力を \f$ v=-R^{-1} B^T P z \f$ として与える行列 \f$ P \f$ を返す。
 */
Eigen::MatrixXd PYBIND11_EXPORT ArimotoPotter(const Eigen::MatrixXd& A, const Eigen::MatrixXd& B, const Eigen::MatrixXd& Q, const Eigen::MatrixXd& R);

/**
 * @brief 4段4次のルンゲクッタ法で積分を行う。
 * 
 * @param [in] x0    元の状態量ベクトル
 * @param [in] t0    元の時刻(s)
 * @param [in] dt    進める時間(s)
 * @param [in] deriv x,tにおける状態量の導関数値を計算するを返す関数オブジェクト
 * 
 * @returns t=t0+dtにおける状態量ベクトル
 */
Eigen::VectorXd PYBIND11_EXPORT RK4(const Eigen::VectorXd& x0,const Time& t0,double dt,std::function<Eigen::VectorXd (const Eigen::VectorXd& x,const Time& t)> deriv);

void exportMathUtility(py::module &m);

ASRC_NAMESPACE_END(util)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
