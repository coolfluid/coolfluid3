// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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
struct TableRowWrapper
{
  typedef common::Table<ValueT> TableT;
  typedef typename TableT::Row RowT;

  static void set_item(RowT& self, const Uint i, const ValueT value)
  {
    if(i >= self.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table row of size " + boost::lexical_cast<std::string>(self.size()));
    self[i] = value;
  }

  static ValueT get_item(RowT& self, const Uint i)
  {
    if(i >= self.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table row of size " + boost::lexical_cast<std::string>(self.size()));
    return self[i];
  }

  static Uint size(const RowT& self)
  {
    return self.size();
  }

  static std::string to_str(RowT& self)
  {
    std::stringstream output;
    common::print_vector(output, self);
    return output.str();
  }
};

/// Tables can be used as python lists
template<typename ValueT>
struct TableList : PythonListInterface
{
  typedef common::Table<ValueT> TableT;
  typedef typename TableT::Row RowT;

  TableList(ComponentWrapper& wrapped) :
    m_table(wrapped.component<TableT>())
  {
  }

  virtual Uint len() const
  {
    return m_table.size();
  }

  virtual object get_item(const Uint i) const
  {
    if(i >= m_table.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(m_table.size()));
    return object(m_table[i]);
  }

  virtual void set_item(const Uint i, object& value)
  {
    list& values = static_cast<list&>(value);
    const Uint nb_values = boost::python::len(values);

    if(i >= m_table.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(m_table.size()));
    if(nb_values != m_table.row_size())
      throw common::BadValue(FromHere(), "Row size " + boost::lexical_cast<std::string>(nb_values) + " does not match row size for Table: " + boost::lexical_cast<std::string>(m_table.row_size()));

    RowT row = m_table[i];
    for(Uint j = 0; j != nb_values; ++j)
      row[j] = extract<ValueT>(values[j]);
  }

  virtual std::string to_str() const
  {
    std::stringstream out_stream;
    out_stream << m_table;
    return out_stream.str();
  }

  TableT& m_table;
};

/// Extra methods for Table
template<typename ValueT>
struct TableMethods
{
  static Uint row_size(ComponentWrapper& wrapped)
  {
    return wrapped.component< common::Table<ValueT> >().row_size();
  }

  static void resize(ComponentWrapper& wrapped, const Uint nb_rows)
  {
    wrapped.component< common::Table<ValueT> >().resize(nb_rows);
  }

  static void set_row_size(ComponentWrapper& wrapped, const Uint nb_cols)
  {
    wrapped.component< common::Table<ValueT> >().set_row_size(nb_cols);
  }
};

template<typename ValueT>
void add_ctable_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj)
{
  if(dynamic_cast<const common::Table<ValueT>*>(&wrapped.component()))
  {
    // Add list functionality
    typedef TableList<ValueT> ListInterfaceT;
    wrapped.set_list_interface(new ListInterfaceT(wrapped));

    // Extra methods
    typedef TableMethods<ValueT> ExtraMethodsT;
    add_function(py_obj, ExtraMethodsT::row_size, "row_size", "Return the number of columns the table can hold");
    add_function(py_obj, ExtraMethodsT::resize, "resize", "Set the size of the table, i.e. the number of rows");
    add_function(py_obj, ExtraMethodsT::set_row_size, "set_row_size", "Set the size of a row, i.e. the number of columns in the table");
  }
}

void add_ctable_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj)
{
  add_ctable_methods<Real>(wrapped, py_obj);
  add_ctable_methods<Uint>(wrapped, py_obj);
}

template<typename ValueT>
void def_ctable_types()
{
  // The Table row types are statically defined
  typedef TableRowWrapper<ValueT> WrapperT;
  scope ctable_row = class_<typename common::Table<ValueT>::Row>(("TableRow_"+common::class_name<ValueT>()).c_str(), "Coolfluid Table row class wrapper", no_init)
    .def("__setitem__", WrapperT::set_item)
    .def("__getitem__", WrapperT::get_item)
    .def("__len__", WrapperT::size)
    .def("__str__", WrapperT::to_str);
}

void def_ctable_types()
{
  def_ctable_types<Real>();
  def_ctable_types<Uint>();
}

} // python
} // cf3
