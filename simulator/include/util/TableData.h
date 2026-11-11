// Copyright (c) 2025-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
//
// テーブルデータを扱うためのユーティリティクラス。
//
#pragma once
#include "../Common.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eigen.h>
#include <Eigen/Core>
#include <Eigen/CXX11/Tensor>
#include "../traits/eigen.h"
#include "../MathUtility.h"

namespace py=pybind11;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
ASRC_INLINE_NAMESPACE_BEGIN(util)

struct PYBIND11_EXPORT TableDataBase{
    virtual TableDataBase* copyForPY() const=0;
    virtual py::object getPointsPY()=0;
    virtual py::object getValuesPY()=0;
    virtual void setDataPY(const py::args& args)=0;
    virtual void setPointsPY(const py::args& args)=0;
    virtual void setValuesPY(const py::object& obj)=0;
    virtual bool check(bool throwIfFailed=true) const=0;
    virtual py::object callPY(py::object& x,bool clamp=false) const=0;
    //virtual py::object callPY(const double& x,bool clamp=false) const=0;
    virtual py::object gradPY(py::object& x,bool clamp=false) const=0;
    //virtual py::object gradPY(const double& x,bool clamp=false) const=0;
};

/**
 * @class TableData
 * @brief 各要素がV次元テンソルの値を持つN次元(N変数)テーブルデータを表すクラス
 * 
 * @tparam Scalar               対応するスカラー型。通常はdoubleかfloat
 * @tparam NumTableDimensions   テーブルの変数の数N。1以上。
 * @tparam NumValueDimensions   テーブルの各要素の値の次元数V。0以上。0のときはスカラー値を意味する。
 * @tparam TableLayout_         テーブルデータを格納する際のLayout。Eigen::RowMajor又はEigen::ColMajor。省略時はEigen::RowMajorとする。
 * @tparam ReturnLayout_        テーブルから得た値を返す際のLayout。Eigen::RowMajor又はEigen::ColMajor。省略時はEigen::ColMajorとする。
 * 
 * @details
 *          @par Layoutについて
 *              RowMajorは最後の軸に沿って連続、最初の軸が最外周、ColMajorは最初の軸に沿って連続、最後の軸が最外周。
 *              EigenはColMajorがデフォルトだが、テーブルデータとして参照するにはRowMajorの方が都合がよい。
 *              テンプレート引数で指定可能とするが、デフォルトではテーブルをRowMajorで保持し、返り値をColMajorで返すものとしている。
 * 
 *          @par 与えるデータ形式について
 *              各変数のデータ点を表すpointsと、実際のテーブルを表すvaluesの2つを与える。
 *              pointsはN個のEigen::VectorXを持つstd::arrayとする。setPoints()やsetData()の第2引数以降にN個の引数を与えることで初期化する。各引数は、VectorXのコンストラクタに与えられるものならば何でもよい。
 *              valuesはN+V次元のテンソルとする。setValues()やsetData()の第1引数に一つの引数を与えることで初期化する。先頭N次元がテーブルのN個の変数に対応し、後のV次元が各要素の値を表すものとする。引数は次元さえあっていればTensorでもMatrixでもVectorでもよい。
 * 
 *          @par 使い方 (Usage)
 *              C++での使い方は以下の通り。
 *              Pythonからもアクセス可能で、numpy.ndarrayを引数として与えることができる。
 *              @code
 *              asrc::core::util::TableData<double,2> table; //2D table with scalar element, RowMajor table, ColMajor return
 *              table.setData(
 *                  Eigen::Tensor<double,2>(3,4).setValues({
 *                      {0,1,2,3},
 *                      {10,11,12,13},
 *                      {20,21,22,23}
 *                  }), //values
 *                  Eigen::VectorXd{{0,1,2}}, //data points of the 1st axis
 *                  Eigen::VectorXd{{-10,0,10}} //data points of the 2nd axis
 *              );
 * 
 *              //
 *              // interpolation
 *              //
 * 
 *              // When the second argument is false or omitted, the value is extrapolated if the given point is outside the table.
 *              double value=table(Eigen::VectorXd{{1.5,5.0}});
 * 
 *              // When the second argument is true, the value will be clamped at the edge of the table point space.
 *              double clamped=table(Eigen::VectorXd{{-100,-100}},true); // returned value is the value at (0,-10)
 * 
 *              // You can interpolate at multiple points by giving a higher-rank tensor/matrix whose last dimension means each data point.
 *              Eigen::VectorXd multi=table(Eigen::MatrixXd{
 *                  {0,0},
 *                  {1,1},
 *                  {2,2}
 *              }); // returned value is 3-element vector
 * 
 *              //
 *              // gradient
 *              //
 * 
 *              // clamping is performed as same as interpolation.
 *              // i-th element of returned value is partial derivative along the i-th axis.
 *              Eigen::VectorXd grad=table.grad(Eigen::VectorXd{{1.5,5.0}});
 * 
 *              Eigen::MatrixXd multigrad=table.grad(Eigen::MatrixXd{
 *                  {0,0},
 *                  {1,1},
 *                  {2,2}
 *              }); // returned value is 3x2 matrix and each row means gradient at each point
 *              @endcode
 */
template<typename Scalar,std::size_t NumTableDimensions,int NumValueDimensions=0, int TableLayout_=Eigen::RowMajor, int ReturnLayout_=Eigen::ColMajor>
struct PYBIND11_EXPORT TableData: TableDataBase{
    static constexpr bool IsTableRowMajor=(TableLayout_ & Eigen::RowMajor)==Eigen::RowMajor;
    static constexpr bool IsReturnRowMajor=(ReturnLayout_ & Eigen::RowMajor)==Eigen::RowMajor;
    static constexpr int TableLayout = IsTableRowMajor ? Eigen::RowMajor : Eigen::ColMajor;
    static constexpr int ReturnLayout = IsReturnRowMajor ? Eigen::RowMajor : Eigen::ColMajor;

    using Type=TableData<Scalar,NumTableDimensions,NumValueDimensions,TableLayout_,ReturnLayout_>;
    using TensorType=Eigen::Tensor<Scalar,NumTableDimensions+NumValueDimensions,TableLayout>;
    using TensorTraits=Eigen::internal::traits<TensorType>;

    std::array<Eigen::VectorX<Scalar>,NumTableDimensions> points;
    TensorType values;

    TableData(){}

    template<typename TableType, typename PointType, typename... OtherPointTypes>
    TableData(TableType&& _values, PointType&& _firstPoints, OtherPointTypes&&... _otherPoints){
        setValues(std::forward<TableType>(_values));
        setPoints(std::forward<PointType>(_firstPoints),std::forward<OtherPointTypes>(_otherPoints)...);
    }

    template<typename TableType, typename PointType, typename... OtherPointTypes>
    void setData(TableType&& _values, PointType&& _firstPoints, OtherPointTypes&&... _otherPoints){
        setValues(std::forward<TableType>(_values));
        setPoints(std::forward<PointType>(_firstPoints),std::forward<OtherPointTypes>(_otherPoints)...);
    }

    template<typename TableType,typename RawType=std::remove_cv_t<std::remove_reference_t<TableType>>>
    requires (
        traits::NumDimensions<RawType> == NumTableDimensions+NumValueDimensions
        && std::convertible_to<traits::Scalar<RawType>,Scalar>
    )
    void setValues(TableType&& _values){
        if constexpr (std::same_as<traits::Scalar<RawType>,Scalar>){
            values=toTensor<TableLayout>(std::forward<TableType>(_values));
        }else{
            values=toTensor<TableLayout>(std::forward<TableType>(_values)).template cast<Scalar>();
        }
    }

    template<traits::matrix TableType,typename RawType=std::remove_cv_t<std::remove_reference_t<TableType>>>
    requires (
        traits::NumDimensions<RawType> == 2
        && NumTableDimensions+NumValueDimensions == 1
        && std::convertible_to<traits::Scalar<RawType>,Scalar>
    )
    void setValues(TableType&& _values){
        using VectorType=std::conditional_t<
            traits::isRowMajor<RawType>,
            Eigen::Matrix<Scalar,1,-1,Eigen::RowMajor>,
            Eigen::VectorX<Scalar>
        >;
        setValues(Eigen::Ref<VectorType>(std::forward<TableType>(_values)));
    }

    void setPoints(const std::array<Eigen::VectorX<Scalar>,NumTableDimensions>& _points){
        points=_points;
    }
    void setPoints(std::array<Eigen::VectorX<Scalar>,NumTableDimensions>&& _points){
        points=std::move(_points);
    }
    template<typename... PointTypes>
    requires (
        sizeof...(PointTypes) >= 1
        && (std::constructible_from<Eigen::VectorX<Scalar>,PointTypes&&> && ...)
    )
    void setPoints(PointTypes&&... _points){
        setPoints(std::array<Eigen::VectorX<Scalar>,NumTableDimensions>{std::forward<PointTypes>(_points)...});
    }

    /**
     * @brief テーブルデータが正しく設定されているかどうかを返す。
     * 
     * @param [in] throwIfFailed 正しくない場合に例外を投げるかどうか。省略時は投げる。
     */
    virtual bool check(bool throwIfFailed=true) const override{
        for(std::size_t i=0;i<NumTableDimensions;++i){
            if(points[i].size()!=values.dimension(i)){
                // 要素数が同じか
                if(throwIfFailed){
                    throw std::runtime_error("The dimension "+std::to_string(i)+" mismatch between points and values.");
                }else{
                    return false;
                }
            }else if (points[i].size()<2){
                // 要素数が2以上か
                if(throwIfFailed){
                    throw std::runtime_error("The dimension "+std::to_string(i)+" has less than 2 points. At least 2 points are needed.");
                }else{
                    return false;
                }
            }
        }
        return true;
    }

    template<typename PointType>
    requires (
        (
            traits::NumDimensions<PointType> >= 1
            || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
        )
        && std::convertible_to<traits::Scalar<PointType>,Scalar>
    )
    auto operator()(const PointType& x,bool clamp=false) const{
        return interpn<ReturnLayout>(points,values,x,clamp);
    }

    template<typename PointType>
    requires (
        (
            traits::NumDimensions<PointType> >= 1
            || (traits::NumDimensions<PointType> == 0 && NumTableDimensions == 1)
        )
        && std::convertible_to<traits::Scalar<PointType>,Scalar>
    )
    auto grad(const PointType& x,bool clamp=false) const{
        return interpgradn<ReturnLayout>(points,values,x,clamp);
    }

    // Python側から利用するための関数群
    virtual TableDataBase* copyForPY() const override{
        return new Type(values,points);
    }
    virtual py::object getPointsPY() override{
        return py::cast(points,py::return_value_policy::reference_internal,py::cast(*this,py::return_value_policy::reference));
    }
    virtual py::object getValuesPY() override{
        return py::cast(values,py::return_value_policy::reference_internal,py::cast(*this,py::return_value_policy::reference));
    }
    virtual void setDataPY(const py::args& args) override{
        if(py::len(args)<2){
            throw std::runtime_error("TableData.setData() requires at least 2 arguments.");
        }
        setValuesPY(args[0]);
        setPointsPY(args[py::slice(1,py::len(args),1)]);
    }
    virtual void setPointsPY(const py::args& args) override{
        if(py::len(args)==NumTableDimensions){
            std::array<Eigen::VectorX<Scalar>,NumTableDimensions> arr;
            int i=0;
            for(auto arg : args){
                try{
                    arr[i]=py::cast<Eigen::VectorX<Scalar>>(arg);
                    ++i;
                }catch(...){
                    if(NumTableDimensions==1){
                        if(py::len(args[0])==NumTableDimensions){
                            try{
                                setPoints(py::cast<std::array<Eigen::VectorX<Scalar>,NumTableDimensions>>(args[0]));
                                return;
                            }catch(...){
                                throw std::runtime_error("Elements of the argument sequence could not be casted to Eigen::VectorX.");
                            }
                        }else{
                            throw std::runtime_error("The length of argument does not equal to NumTableDimensions ("+std::to_string(NumTableDimensions)+")");
                        }
                    }else{
                        throw std::runtime_error("TableData.setPoints() only accept NumPy arrays or a single sequence of them.");
                    }
                }
            }
            setPoints(std::move(arr));
            return;
        }else if(py::len(args)==1){
            if(py::len(args[0])==NumTableDimensions){
                try{
                    setPoints(py::cast<std::array<Eigen::VectorX<Scalar>,NumTableDimensions>>(args[0]));
                    return;
                }catch(...){
                    throw std::runtime_error("Elements of the argument sequence could not be casted to Eigen::VectorX.");
                }
            }else{
                throw std::runtime_error("The length of argument does not equal to NumTableDimensions ("+std::to_string(NumTableDimensions)+")");
            }
        }else{
            throw std::runtime_error("The length of argument does not equal to NumTableDimensions ("+std::to_string(NumTableDimensions)+")");
        }
        
    }
    virtual void setValuesPY(const py::object& obj) override{
        setValues(py::cast<TensorType>(obj));
    }
    virtual py::object callPY(py::object& x,bool clamp=false) const override{
        if constexpr (NumTableDimensions==1){
            // NumTableDimensionsが1のときはスカラーを直接与えてもよい。
            try{
                return py::cast(this->operator()(py::cast<Scalar>(x)));
            }catch(const py::cast_error& e){
                //pass
            }
        }

        // そうでなければnumpy.ndarrayを挟む。
        py::array_t<Scalar> arr;
        if(py::isinstance<py::array>(x)){
            // はじめからnumpy.ndarrayの場合
            arr=py::array_t<Scalar>::ensure(x);
        }else{
            // numpy.ndarrayへの変換が必要な場合
            auto np=py::module_::import("numpy");
            arr=py::array_t<Scalar>(np.attr("array")(x));
        }
        if(arr.ndim()==0){
            if constexpr (NumTableDimensions==1){
                return py::cast(this->operator()(py::cast<Scalar>(arr.reshape(std::array<py::ssize_t,1>{1})[0])));
            }else{
                throw std::runtime_error("TableData accept scalar argument only when NumTableDimensions==1.");
            }
        }else if(arr.ndim()==1){
            return py::cast(this->operator()(py::cast<Eigen::Tensor<Scalar,1,Eigen::RowMajor>>(arr),clamp));
        }else{
            std::vector<py::ssize_t> return_shape;
            for(py::ssize_t i=0;i<arr.ndim()-1;++i){
                return_shape.push_back(arr.shape(i));
            }
            std::vector<py::ssize_t> flattened_shape{-1,arr.shape(arr.ndim()-1)};
            py::array ret=py::cast(this->operator()(py::cast<Eigen::Tensor<Scalar,2,Eigen::RowMajor>>(arr.reshape(flattened_shape)),clamp));
            for(py::ssize_t i=1;i<ret.ndim();++i){
                return_shape.push_back(ret.shape(i));
            }
            return std::move(ret).reshape(return_shape);
        }
    }
    /*
    virtual py::object callPY(const double& x,bool clamp=false) const override{
        if constexpr (NumTableDimensions==1){
            return py::cast(this->operator()(x,clamp));
        }else{
            throw std::runtime_error("TableData accept scalar argument only when NumTableDimensions==1.");
        }
    }
    */
    virtual py::object gradPY(py::object& x,bool clamp=false) const override{
        if constexpr (NumTableDimensions==1){
            // NumTableDimensionsが1のときはスカラーを直接与えてもよい。
            try{
                return py::cast(grad(py::cast<Scalar>(x),clamp));
            }catch(const py::cast_error& e){
                //pass
            }
        }

        // そうでなければnumpy.ndarrayを挟む。
        py::array_t<Scalar> arr;
        if(py::isinstance<py::array>(x)){
            // はじめからnumpy.ndarrayの場合
            arr=py::array_t<Scalar>::ensure(x);
        }else{
            // numpy.ndarrayへの変換が必要な場合
            auto np=py::module_::import("numpy");
            arr=py::array_t<Scalar>(np.attr("array")(x));
        }
        if(arr.ndim()==0){
            if constexpr (NumTableDimensions==1){
                return py::cast(grad(py::cast<Scalar>(arr.reshape(std::array<py::ssize_t,1>{1})[0]),clamp));
            }else{
                throw std::runtime_error("TableData accept scalar argument only when NumTableDimensions==1.");
            }
        }else if(arr.ndim()==1){
            return py::cast(grad(py::cast<Eigen::Tensor<Scalar,1,Eigen::RowMajor>>(arr),clamp));
        }else{
            std::vector<py::ssize_t> return_shape;
            for(py::ssize_t i=0;i<arr.ndim()-1;++i){
                return_shape.push_back(arr.shape(i));
            }
            std::vector<py::ssize_t> flattened_shape{-1,arr.shape(arr.ndim()-1)};
            py::array ret=py::cast(grad(py::cast<Eigen::Tensor<Scalar,2,Eigen::RowMajor>>(arr.reshape(flattened_shape)),clamp));
            for(py::ssize_t i=1;i<ret.ndim();++i){
                return_shape.push_back(ret.shape(i));
            }
            return std::move(ret).reshape(return_shape);
        }
    }
    /*
    virtual py::object gradPY(const double& x,bool clamp=false) const override{
        if constexpr (NumTableDimensions==1){
            return py::cast(grad(x,clamp));
        }else{
            throw std::runtime_error("TableData accept scalar argument only when NumTableDimensions==1.");
        }
    }
    */
    template<class Archive>
    void serialize(Archive & archive){
        archive(points);
        archive(values);
    }
};

struct PYBIND11_EXPORT PyTableData: TableDataBase{
    PyTableData(const PyTableData& other);
    PyTableData(PyTableData&& other);
    PyTableData(TableDataBase& _data);
    PyTableData(const TableDataBase& _data);
    PyTableData(TableDataBase&& _data);
    PyTableData(const TableDataBase* _data,bool _isOwner);
    ~PyTableData();
    virtual TableDataBase* copyForPY() const override;
    virtual py::object getPointsPY() override;
    virtual py::object getValuesPY() override;
    virtual void setDataPY(const py::args& args) override;
    virtual void setPointsPY(const py::args& args) override;
    virtual void setValuesPY(const py::object& obj) override;
    virtual bool check(bool throwIfFailed=true) const override;
    virtual py::object callPY(py::object& x,bool clamp=false) const override;
    //virtual py::object callPY(const double& x,bool clamp=false) const override;
    virtual py::object gradPY(py::object& x,bool clamp=false) const override;
    //virtual py::object gradPY(const double& x,bool clamp=false) const override;
    TableDataBase* data_ptr;
    bool isOwner;
};

/**
 * @brief Python側でTableDataのインスタンスを生成するための関数。
 * 
 * @param [in] NumTableDimensions テーブルの変数の数(1〜6)
 * @param [in] NumValueDimensions テーブルの各要素の値の次元(0〜2)
 * 
 * @returns Python側で使用するためのラッパーインスタンス
 * 
 * @details
 *          C++側でテンプレートとして実装されているものを非テンプレート関数として呼び出す必要があるため、
 *          関数の内部でインスタンス化したもののみに対応する。
 *          現時点では、Scalarはdoubleのみ、NumTableDimensionsは1〜6、NumValueDimensionsは0〜2とする。
 */
PyTableData createPyTableData(int NumTableDimensions,int NumValueDimensions);

// Explicit instantiation
#define ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_IMPL(Scalar,NTD,NVD,_EXTERN) \
_EXTERN template struct TableData<Scalar,NTD,NVD>; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::operator()<Eigen::VectorXd>(const Eigen::VectorXd&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::grad<Eigen::VectorXd>(const Eigen::VectorXd&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::operator()<Eigen::Tensor<Scalar,1,Eigen::RowMajor>>(const Eigen::Tensor<Scalar,1,Eigen::RowMajor>&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::grad<Eigen::Tensor<Scalar,1,Eigen::RowMajor>>(const Eigen::Tensor<Scalar,1,Eigen::RowMajor>&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::operator()<Eigen::MatrixXd>(const Eigen::MatrixXd&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::grad<Eigen::MatrixXd>(const Eigen::MatrixXd&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::operator()<Eigen::Tensor<Scalar,2,Eigen::RowMajor>>(const Eigen::Tensor<Scalar,2,Eigen::RowMajor>&,bool) const; \
_EXTERN template auto PYBIND11_EXPORT TableData<Scalar,NTD,NVD>::grad<Eigen::Tensor<Scalar,2,Eigen::RowMajor>>(const Eigen::Tensor<Scalar,2,Eigen::RowMajor>&,bool) const; \

#define ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(Scalar,NTD,NVD) ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_IMPL(Scalar,NTD,NVD,)
#define ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(Scalar,NTD,NVD) ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_IMPL(Scalar,NTD,NVD,extern)

ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,1,0)
extern template auto TableData<double,1,0>::operator()<double>(const double&,bool) const;
extern template auto TableData<double,1,0>::grad<double>(const double&,bool) const;
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,2,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,3,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,4,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,5,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,6,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,1,1)
extern template auto TableData<double,1,1>::operator()<double>(const double&,bool) const;
extern template auto TableData<double,1,1>::grad<double>(const double&,bool) const;
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,2,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,3,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,4,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,5,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,6,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,1,2)
extern template auto TableData<double,1,2>::operator()<double>(const double&,bool) const;
extern template auto TableData<double,1,2>::grad<double>(const double&,bool) const;
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,2,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,3,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,4,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,5,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION_EXTERN(double,6,2)

void exportTableData(py::module &m);

ASRC_NAMESPACE_END(util)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)


PYBIND11_NAMESPACE_BEGIN(PYBIND11_NAMESPACE)
    namespace detail{
        template<typename Scalar,std::size_t NumTableDimensions,int NumValueDimensions>
        struct type_caster<asrc::core::TableData<Scalar,NumTableDimensions,NumValueDimensions>>
        {
        public:
            using TableDataType=asrc::core::TableData<Scalar,NumTableDimensions,NumValueDimensions>;
            static constexpr auto name = _("TableData");

            template <typename T>
            using cast_op_type = ::pybind11::detail::cast_op_type<T>;

            bool load(handle src, bool)
            {
                try {
                    asrc::core::PyTableData* wrapper=src.cast<asrc::core::PyTableData*>();
                    if(!wrapper){
                        return false;
                    }
                    value=dynamic_cast<TableDataType*>(wrapper->data_ptr);
                    return bool(value);
                }
                catch (...)
                {
                    return false;
                }
            }

            static handle cast(const TableDataType& src, return_value_policy policy , handle parent){
                if(
                    policy==return_value_policy::automatic
                    || policy==return_value_policy::automatic_reference
                ){
                    policy = return_value_policy::copy;
                }
                return cast(std::addressof(src), policy, parent);
            }
            static handle cast(TableDataType&& src, return_value_policy policy , handle parent){
                return cast(std::addressof(src), return_value_policy::move, parent);
            }

            static handle cast(const TableDataType* src, return_value_policy policy , handle parent){
                if (src == nullptr) {
                    return none().release();
                }
                bool isOwner;

                switch (policy) {
                    case return_value_policy::automatic:
                    case return_value_policy::take_ownership:
                        isOwner=true;
                        break;

                    case return_value_policy::automatic_reference:
                    case return_value_policy::reference:
                        isOwner=false;
                        break;

                    case return_value_policy::copy:
                        isOwner=true;
                        break;

                    case return_value_policy::move:
                        isOwner=true;
                        break;

                    case return_value_policy::reference_internal:
                        isOwner=false;
                        break;

                    default:
                        throw cast_error("unhandled return_value_policy: should not happen!");
                }
                
                object inst = py::cast(asrc::core::PyTableData(src,isOwner),return_value_policy::move,none());
                if(policy==return_value_policy::reference_internal){
                    keep_alive_impl(inst, parent);
                }
                return inst.release();
            }

            operator TableDataType *() { return value; }
            operator TableDataType &() {
                if(!value){
                    throw reference_cast_error();
                }
                return *value;
            }

            protected:
            TableDataType* value;
        };
    }
PYBIND11_NAMESPACE_END(PYBIND11_NAMESPACE)
