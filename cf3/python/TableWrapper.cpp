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

#include "common/Table.hpp"

#include "python/ComponentWrapper.hpp"
#include "python/TableWrapper.hpp"
#include "python/Utility.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

/// Functions exposed to python dealing with table rows
template<typename ValueT>
struct TableRowWrapperConst
{
  typedef common::Table<ValueT> TableT;
  typedef typename TableT::ConstRow ConstRowT;

  static ValueT get_item(const ConstRowT& self, const Uint i)
  {
    if(i >= self.size())
    {
      PyErr_SetString(PyExc_IndexError, ("Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table row of size " + boost::lexical_cast<std::string>(self.size())).c_str());
      boost::python::throw_error_already_set();
    }
    return self[i];
  }

  static Uint size(const ConstRowT& self)
  {
    return self.size();
  }

  static std::string to_str(const ConstRowT& self)
  {
    std::stringstream output;
    common::print_vector(output, self);
    return output.str();
  }
};

template<typename ValueT>
struct TableRowWrapper : public TableRowWrapperConst<ValueT>
{
  typedef common::Table<ValueT> TableT;
  typedef typename TableT::Row RowT;

  static void set_item(RowT& self, const Uint i, const ValueT value)
  {
    if(i >= self.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table row of size " + boost::lexical_cast<std::string>(self.size()));
    self[i] = value;
  }
};

/// Extra methods for Table
template<typename ValueT>
struct TableMethods
{
  typedef common::Table<ValueT> TableT;

  static Uint row_size(const ComponentWrapperBase& wrapped)
  {
    return wrapped.component<TableT>().row_size();
  }

  static void resize(ComponentWrapper& wrapped, const Uint nb_rows)
  {
    wrapped.component<TableT>().resize(nb_rows);
  }

  static void set_row_size(ComponentWrapper& wrapped, const Uint nb_cols)
  {
    wrapped.component<TableT>().set_row_size(nb_cols);
  }

  static Uint len(const ComponentWrapperBase& wrapped)
  {
    return wrapped.component<TableT>().size();
  }

  static object get_item(ComponentWrapper& wrapped, const Uint i)
  {
    TableT& table = wrapped.component<TableT>();
    if(i >= table.size())
    {
      PyErr_SetString(PyExc_IndexError, ("Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(table.size())).c_str());
      boost::python::throw_error_already_set();
    }
    return object(table[i]);
  }

  static object get_item_const(ComponentWrapperConst& wrapped, const Uint i)
  {
    const TableT& table = wrapped.component<TableT const>();
    if(i >= table.size())
    {
      PyErr_SetString(PyExc_IndexError, ("Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(table.size())).c_str());
      boost::python::throw_error_already_set();
    }
    return object(table[i]);
  }

  static void set_item(ComponentWrapper& wrapped, const Uint i, object& value)
  {
    list& values = static_cast<list&>(value);
    const Uint nb_values = boost::python::len(values);
    TableT& table = wrapped.component<TableT>();

    if(i >= table.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(table.size()));
    if(nb_values != table.row_size())
      throw common::BadValue(FromHere(), "Row size " + boost::lexical_cast<std::string>(nb_values) + " does not match row size for Table: " + boost::lexical_cast<std::string>(table.row_size()));

    typename TableT::Row row = table[i];
    for(Uint j = 0; j != nb_values; ++j)
      row[j] = extract<ValueT>(values[j]);
  }

  static std::string to_str(const ComponentWrapperBase& wrapped)
  {
    std::stringstream out_stream;
    out_stream << wrapped.component<TableT>();
    return out_stream.str();
  }
};

template<typename ValueT>
void def_ctable_types()
{
  typedef TableRowWrapperConst<ValueT> ConstWrapperT;
  typedef TableRowWrapper<ValueT> WrapperT;

  typedef typename common::Table<ValueT>::Row RowT;
  typedef boost::detail::multi_array::const_sub_array<ValueT,1> ConstRowT; // The const from the CF3 typedef causes trouble here

  class_<ConstRowT>(("TableConstRow_"+common::class_name<ValueT>()).c_str(), "Coolfluid Table row class wrapper, const version", no_init)
    .def("__getitem__", ConstWrapperT::get_item)
    .def("__len__", ConstWrapperT::size)
    .def("__str__", ConstWrapperT::to_str);

  class_<RowT>(("TableRow_"+common::class_name<ValueT>()).c_str(), "Coolfluid Table row class wrapper", no_init)
    .def("__setitem__", WrapperT::set_item)
    .def("__getitem__", ConstWrapperT::get_item)
    .def("__len__", ConstWrapperT::size)
    .def("__str__", ConstWrapperT::to_str);

  boost::python::implicitly_convertible<RowT, ConstRowT>();

  typedef DerivedComponentWrapper< common::Table<ValueT> > TableWrapper;
  typedef DerivedComponentWrapper< common::Table<ValueT> const > TableWrapperConst;

  boost::python::class_<TableWrapper, boost::python::bases<ComponentWrapper> >(("Table_"+common::class_name<ValueT>()).c_str(), boost::python::no_init)
    .def("row_size", TableMethods<ValueT>::row_size, "Return the number of columns the table can hold")
    .def("resize", TableMethods<ValueT>::resize, "Set the size of the table, i.e. the number of rows")
    .def("set_row_size", TableMethods<ValueT>::set_row_size, "Set the size of a row, i.e. the number of columns in the table")
    .def("__setitem__", TableMethods<ValueT>::set_item)
    .def("__getitem__", TableMethods<ValueT>::get_item)
    .def("__len__", TableMethods<ValueT>::len)
    .def("__str__", TableMethods<ValueT>::to_str);

  boost::python::class_<TableWrapperConst, boost::python::bases<ComponentWrapperConst> >(("TableConst_"+common::class_name<ValueT>()).c_str(), boost::python::no_init)
    .def("row_size", TableMethods<ValueT>::row_size, "Return the number of columns the table can hold")
    .def("__getitem__", TableMethods<ValueT>::get_item_const)
    .def("__len__", TableMethods<ValueT>::len)
    .def("__str__", TableMethods<ValueT>::to_str);

  ComponentWrapperRegistry::instance().register_factory< DefaultComponentWrapperFactory< common::Table<ValueT> > >();
}

void def_ctable_types()
{
  def_ctable_types<Real>();
  def_ctable_types<Uint>();
}

} // python
} // cf3
