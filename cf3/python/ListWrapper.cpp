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

/// Tables can be used as python lists
template<typename ValueT>
struct List : PythonListInterface
{
  typedef common::List<ValueT> ListT;

  List(ComponentWrapper& wrapped) :
    m_list(wrapped.component<ListT>())
  {
  }

  virtual Uint len() const
  {
    return m_list.size();
  }

  virtual object get_item(const Uint i) const
  {
    if(i >= m_list.size())
    {
      PyErr_SetString(PyExc_IndexError, ("Index " + boost::lexical_cast<std::string>(i) + " is out of range for List with number of rows: " + boost::lexical_cast<std::string>(m_list.size())).c_str());
      boost::python::throw_error_already_set();
    }
    return object(m_list.array()[i]);
  }

  virtual void set_item(const Uint i, boost::python::object& value)
  {
    if(i >= m_list.size())
      throw common::BadValue(FromHere(), "Index " + boost::lexical_cast<std::string>(i) + " is out of range for Table with number of rows: " + boost::lexical_cast<std::string>(m_list.size()));
    m_list.array()[i] = extract<ValueT>(value);
  }

  virtual std::string to_str() const
  {
    std::stringstream out_stream;
    out_stream << m_list;
    return out_stream.str();
  }

  ListT& m_list;
};

/// Extra methods for Table
template<typename ValueT>
struct ListMethods
{
  static void resize(ComponentWrapper& wrapped, const Uint nb_rows)
  {
    wrapped.component< common::List<ValueT> >().resize(nb_rows);
  }
};

template<typename ValueT>
void add_clist_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj)
{
  if(dynamic_cast<const common::List<ValueT>*>(&wrapped.component()))
  {
    // Add list functionality
    typedef List<ValueT> ListInterfaceT;
    wrapped.set_list_interface(new ListInterfaceT(wrapped));

    // Extra methods
    typedef ListMethods<ValueT> ExtraMethodsT;
    add_function(py_obj, ExtraMethodsT::resize, "resize", "Set the size of the table, i.e. the number of rows");
  }
}

void add_clist_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj)
{
  add_clist_methods<Real>(wrapped, py_obj);
  add_clist_methods<Uint>(wrapped, py_obj);
}

template<typename ValueT>
void def_clist_types()
{
}

void def_clist_types()
{
  def_clist_types<Real>();
  def_clist_types<Uint>();
}

} // python
} // cf3
