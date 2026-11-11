// Copyright (c) 2024-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
//
// Eigenに関するtraits
//
#pragma once
#include <type_traits>
#include <concepts>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/CXX11/Tensor>
#include "../util/macros/common_macros.h"

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
ASRC_NAMESPACE_BEGIN(traits)

template<typename MatrixType>
concept matrix = std::derived_from<MatrixType,Eigen::DenseBase<MatrixType>>;

template<typename TensorType>
concept tensor = std::derived_from<TensorType,Eigen::TensorBase<TensorType,Eigen::ReadOnlyAccessors>>;

template<typename TensorType>
concept has_dimensions = requires (TensorType& t) {
    t.dimensions();
};

template<typename TensorType>
concept has_sizes = requires (TensorType& t) {
    t.sizes();
};

template<typename TensorType>
concept has_size = requires (TensorType& t) {
    { t.size() } -> std::integral;
};

template<typename TensorType>
concept isRowMajor = (
    (tensor<TensorType> && static_cast<int>(Eigen::internal::traits<TensorType>::Layout) == static_cast<int>(Eigen::RowMajor))
    || (matrix<TensorType> && static_cast<bool>(TensorType::IsRowMajor))
);

template<typename TensorType>
concept isRawAccessible = (
    (tensor<TensorType> && requires {TensorType::RawAccess;} && static_cast<bool>(TensorType::RawAccess))
    || (matrix<TensorType> && static_cast<bool>(TensorType::IsPlainObjectBase))
);

template<typename ValueType>
inline constexpr int NumDimensions = 0;

template<tensor TensorType>
inline constexpr int NumDimensions<TensorType> = Eigen::internal::traits<TensorType>::NumDimensions;

template<matrix MatrixType>
inline constexpr int NumDimensions<MatrixType> = (MatrixType::RowsAtCompileTime==1 && MatrixType::ColsAtCompileTime==1) ? 0 : (MatrixType::IsVectorAtCompileTime ? 1 : 2);

template<typename ValueType>
inline constexpr int Layout = (isRowMajor<ValueType> ? Eigen::RowMajor : Eigen::ColMajor);

template<typename ValueType>
struct Scalar_impl {
    using type = ValueType;
};
template<typename ValueType>
requires ( tensor<ValueType> || matrix<ValueType> )
struct Scalar_impl<ValueType> {
    using type = typename Eigen::internal::traits<ValueType>::Scalar;
};

template<typename ValueType>
using Scalar = typename Scalar_impl<ValueType>::type;

ASRC_NAMESPACE_END(traits)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)

