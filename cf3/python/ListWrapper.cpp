// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include <sstream>

#include <boost/weak_ptr.hpp>

#include "common/Log.hpp"
#include "common/StreamHelpers.hpp"

#include "common/List.hpp"

#include "python/ComponentWrapper.hpp"
#include "python/ListWrapper.hpp"
#include "python/Utility.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

template<typename ValueT>
struct ListMethods
{
  typedef common::List<ValueT> ListT;
  static void resize(ComponentWrapper& wrapped, const Uint nb_rows)
  {
    wrapped.component<ListT>().resize(nb_rows);
  }

  static Uint len(const ComponentWrapperBase& wrapped)
  {
    return wrapped.component<ListT>().size();
  }

  static object get_item(ComponentWrapperBase& wrapped, const Uint i)
  {
    const ListT& list = wrapped.component<ListT>();
    if(i >= list.size())
    {
      PyErr_SetString(PyExc_IndexError, ("Index " + boost::lexical_cast<std::string>(i) + " is out of range for List with number of rows: " + boost::lexical_cast<std::string>(list.size())).c_str());
      boost::python::throw_error_already_set();
    }
    return object(list.array()[i]);
  }

  static void set_item(ComponentWrapper& wrapped, const Uint i, object& value)
  {
    ListT& list = wrapped.component<ListT>();
    if(i >= list.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(list.size()));
    list.array()[i] = extract<ValueT>(value);
  }

  static std::string to_str(const ComponentWrapperBase& wrapped)
  {
    std::stringstream out_stream;
    out_stream << wrapped.component<ListT>();
    return out_stream.str();
  }

};

template<typename ValueT>
void def_clist_types()
{
  typedef DerivedComponentWrapper< common::List<ValueT> > ListWrapper;
  typedef DerivedComponentWrapper< common::List<ValueT> const > ListWrapperConst;

  boost::python::class_<ListWrapper, boost::python::bases<ComponentWrapper> >(("List_"+common::class_name<ValueT>()).c_str(), boost::python::no_init)
    .def("resize", ListMethods<ValueT>::resize, "Set the size of the List, i.e. the number of rows")
    .def("__setitem__", ListMethods<ValueT>::set_item)
    .def("__getitem__", ListMethods<ValueT>::get_item)
    .def("__len__", ListMethods<ValueT>::len)
    .def("__str__", ListMethods<ValueT>::to_str);

  boost::python::class_<ListWrapperConst, boost::python::bases<ComponentWrapperConst> >(("ListConst_"+common::class_name<ValueT>()).c_str(), boost::python::no_init)
    .def("__getitem__", ListMethods<ValueT>::get_item)
    .def("__len__", ListMethods<ValueT>::len)
    .def("__str__", ListMethods<ValueT>::to_str);

  ComponentWrapperRegistry::instance().register_factory< DefaultComponentWrapperFactory< common::List<ValueT> > >();
}

void def_clist_types()
{
  def_clist_types<Real>();
  def_clist_types<Uint>();
  def_clist_types<bool>();
}

} // python
} // cf3
