// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>

#include <sstream>

#include <boost/weak_ptr.hpp>

#include "common/Log.hpp"
#include "common/StreamHelpers.hpp"

#include "Mesh/CTable.hpp"

#include "Python/Component.hpp"
#include "Python/CTable.hpp"
#include "Python/Utility.hpp"

namespace cf3 {
namespace Python {

using namespace boost::python;

/// Functions exposed to python dealing with table rows
template<typename ValueT>
struct CTableRowWrapper
{
  typedef Mesh::CTable<ValueT> TableT;
  typedef typename TableT::Row RowT;

  static void set_item(RowT& self, const Uint i, const ValueT value)
  {
    if(i >= self.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for CTable row of size " + boost::lexical_cast<std::string>(self.size()));
    self[i] = value;
  }

  static ValueT get_item(RowT& self, const Uint i)
  {
    if(i >= self.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for CTable row of size " + boost::lexical_cast<std::string>(self.size()));
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

/// CTables can be used as python lists
template<typename ValueT>
struct CTableList : PythonListInterface
{
  typedef Mesh::CTable<ValueT> TableT;
  typedef typename TableT::Row RowT;

  CTableList(ComponentWrapper& wrapped) :
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
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for CTable with number of rows: " + boost::lexical_cast<std::string>(m_table.size()));
    return object(m_table[i]);
  }

  virtual void set_item(const Uint i, object& value)
  {
    list& values = static_cast<list&>(value);
    const Uint nb_values = boost::python::len(values);

    if(i >= m_table.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for CTable with number of rows: " + boost::lexical_cast<std::string>(m_table.size()));
    if(nb_values != m_table.row_size())
      throw common::BadValue(FromHere(), "Row size " + boost::lexical_cast<std::string>(nb_values) + " does not match row size for CTable: " + boost::lexical_cast<std::string>(m_table.row_size()));

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

/// Extra methods for CTable
template<typename ValueT>
struct CTableMethods
{
  static Uint row_size(ComponentWrapper& wrapped)
  {
    return wrapped.component< Mesh::CTable<ValueT> >().row_size();
  }
};

template<typename ValueT>
void add_ctable_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj)
{
  if(dynamic_cast<const Mesh::CTable<ValueT>*>(&wrapped.component()))
  {
    CFdebug << "adding custom methods for " << Mesh::CTable<ValueT>::type_name() << CFendl;

    // Add list functionality
    typedef CTableList<ValueT> ListInterfaceT;
    wrapped.set_list_interface(new ListInterfaceT(wrapped));

    // Extra methods
    typedef CTableMethods<ValueT> ExtraMethodsT;
    add_function(py_obj, ExtraMethodsT::row_size, "row_size", "Return the number of columns the table can hold");
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
  // The CTable row types are statically defined
  typedef CTableRowWrapper<ValueT> WrapperT;
  scope ctable_row = class_<typename Mesh::CTable<ValueT>::Row>(("CTableRow_"+common::class_name<ValueT>()).c_str(), "Coolfluid CTable row class wrapper", no_init)
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

} // Python
} // cf3
