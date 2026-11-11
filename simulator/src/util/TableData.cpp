// Copyright (c) 2025-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#include "util/TableData.h"
namespace py=pybind11;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)
ASRC_INLINE_NAMESPACE_BEGIN(util)

PyTableData::PyTableData(const PyTableData& other){
    if(other.isOwner){
        isOwner=true;
        data_ptr=other.data_ptr->copyForPY();
    }else{
        isOwner=false;
        data_ptr=other.data_ptr;
    }
}
PyTableData::PyTableData(PyTableData&& other){
    isOwner=other.isOwner;
    data_ptr=other.data_ptr;
    other.isOwner=false;
}
PyTableData::PyTableData(TableDataBase& _data){
    isOwner=false;
    data_ptr=&_data;
}
PyTableData::PyTableData(const TableDataBase& _data){
    isOwner=true;
    data_ptr=_data.copyForPY();
}
PyTableData::PyTableData(TableDataBase&& _data){
    isOwner=true;
    data_ptr=_data.copyForPY();
}
PyTableData::PyTableData(const TableDataBase* _data,bool _isOwner):isOwner(_isOwner){
    if(!_data){
        throw std::runtime_error("PyTableData cannot be instantiated with nullptr.");
    }
    if(isOwner){
        data_ptr=_data->copyForPY();
    }else{
        data_ptr=const_cast<TableDataBase*>(_data);
    }
}
PyTableData::~PyTableData(){
    if(isOwner){
        delete data_ptr;
    }
}
TableDataBase* PyTableData::copyForPY() const{
    return new PyTableData(data_ptr,true);
}
py::object PyTableData::getPointsPY(){
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->getPointsPY();
}
py::object PyTableData::getValuesPY(){
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->getValuesPY();
}
void PyTableData::setDataPY(const py::args& args){
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    data_ptr->setDataPY(args);
}
void PyTableData::setPointsPY(const py::args& args){
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    data_ptr->setPointsPY(args);
}
void PyTableData::setValuesPY(const py::object& obj){
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    data_ptr->setValuesPY(obj);
}
bool PyTableData::check(bool throwIfFailed) const{
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->check(throwIfFailed);
}
py::object PyTableData::callPY(py::object& x,bool clamp) const{
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->callPY(x,clamp);
}
/*
py::object PyTableData::callPY(const double& x,bool clamp) const{
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->callPY(x,clamp);
}
*/
py::object PyTableData::gradPY(py::object& x,bool clamp) const{
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->gradPY(x,clamp);
}
/*
py::object PyTableData::gradPY(const double& x,bool clamp) const{
    if(!data_ptr){
        throw std::runtime_error("PyTableData's data_ptr is expired.");
    }
    return data_ptr->gradPY(x,clamp);
}
*/

// Explicit instantiation
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,1,0)
template auto TableData<double,1,0>::operator()<double>(const double&,bool) const;
template auto TableData<double,1,0>::grad<double>(const double&,bool) const;
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,2,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,3,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,4,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,5,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,6,0)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,1,1)
template auto TableData<double,1,1>::operator()<double>(const double&,bool) const;
template auto TableData<double,1,1>::grad<double>(const double&,bool) const;
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,2,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,3,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,4,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,5,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,6,1)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,1,2)
template auto TableData<double,1,2>::operator()<double>(const double&,bool) const;
template auto TableData<double,1,2>::grad<double>(const double&,bool) const;
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,2,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,3,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,4,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,5,2)
ASRC_TABLEDATA_EXPLICIT_INSTANTIATION(double,6,2)


#define ASRC_CREATE_PYTABLEDATA_IMPL(Scalar,NTD,NVD) \
if(NumTableDimensions==NTD && NumValueDimensions==NVD){return PyTableData(TableData<Scalar,NTD,NVD>());} \

PyTableData createPyTableData(int NumTableDimensions,int NumValueDimensions){
    ASRC_CREATE_PYTABLEDATA_IMPL(double,1,0)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,2,0)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,3,0)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,4,0)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,5,0)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,6,0)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,1,1)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,2,1)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,3,1)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,4,1)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,5,1)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,6,1)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,1,2)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,2,2)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,3,2)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,4,2)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,5,2)
    ASRC_CREATE_PYTABLEDATA_IMPL(double,6,2)
    throw std::runtime_error("Currently, only TableData with 1<=NumTableDimensions<=6 and 0<=NumValueDimensions<=2 can be instantiated from Python side.");
}

void exportTableData(py::module &m)
{
    using namespace pybind11::literals;

    py::class_<TableDataBase>(m,"TableDataBase")
    .def_property("points",&TableDataBase::getPointsPY,&TableDataBase::setPointsPY)
    .def_property("values",&TableDataBase::getValuesPY,&TableDataBase::setValuesPY)
    .def("setData",&TableDataBase::setDataPY)
    .def("setPoints",&TableDataBase::setPointsPY)
    .def("setValues",&TableDataBase::setValuesPY)
    DEF_FUNC(TableDataBase,check)
    .def("__call__",py::overload_cast<py::object&,bool>(&TableDataBase::callPY,py::const_)
        ,"x"_a,"clamp"_a=false)
    //.def("__call__",py::overload_cast<const double&,bool>(&TableDataBase::callPY,py::const_)
    //    ,"x"_a,"clamp"_a=false)
    .def("grad",py::overload_cast<py::object&,bool>(&TableDataBase::gradPY,py::const_))
    //.def("grad",py::overload_cast<const double&,bool>(&TableDataBase::gradPY,py::const_))
    ;

    py::class_<PyTableData,TableDataBase>(m,"TableData")
    .def_static("create",&createPyTableData)
    ;
}

ASRC_NAMESPACE_END(util)
ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
