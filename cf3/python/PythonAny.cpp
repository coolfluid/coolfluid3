// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "BoostPython.hpp"
#include "PythonAny.hpp"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include <boost/fusion/support/pair.hpp>

#include "common/Component.hpp"
#include "common/Foreach.hpp"
#include "common/URI.hpp"
#include "common/UUCount.hpp"

#include "python/ComponentWrapper.hpp"

namespace cf3 {
namespace python {

using namespace boost::fusion;

// Types that can be held by any
typedef boost::mpl::vector7<std::string, int, bool, Real, common::URI, common::UUCount, Handle<common::Component> > PythonToAnyTypes;
typedef boost::mpl::vector8<std::string, Uint, int, bool, Real, common::URI, common::UUCount, Handle<common::Component> > AnyToPythonTypes;

/// Mapping type for the built-in types
typedef map
<
  pair<std::string, const PyTypeObject*>,
  pair<int, const PyTypeObject*>,
  pair<bool, const PyTypeObject*>,
  pair<Real, const PyTypeObject*>
> AnyTypeMapT;

/// Actual values of the python type pointers that correspond to each type
const AnyTypeMapT python_type_map
(
  make_pair<std::string>(&PyString_Type),
  make_pair<int>(&PyInt_Type),
  make_pair<bool>(&PyBool_Type),
  make_pair<Real>(&PyFloat_Type)
);

/// Conversion from any to python for basic types
struct AnyToPython
{
  AnyToPython(const boost::any& value, boost::python::object& result) :
  m_value(value),
  m_result(result)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(typeid(T) != m_value.type())
    {
      return;
    }

    m_result = boost::python::object(boost::any_cast<T>(m_value));
  }

  void operator()(const Handle<common::Component>&) const
  {
    if(typeid(Handle<common::Component>) != m_value.type())
    {
      return;
    }

    m_result = boost::python::object( wrap_component(boost::any_cast< Handle<common::Component> >(m_value)));
  }

  const boost::any& m_value;
  boost::python::object& m_result;
};

/// Conversion for lists
struct AnyListToPython
{
  AnyListToPython(const boost::any& value, boost::python::object& result) :
    m_value(value),
    m_result(result)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(typeid(std::vector<T>) != m_value.type())
      return;

    boost::python::list result;
    const std::vector<T> val = boost::any_cast< std::vector<T> >(m_value);
    BOOST_FOREACH(const T& item, val)
    {
      boost::python::object list_item;
      AnyToPython(item, list_item)(item);
      result.append(list_item);
    }

    m_result = result;
  }

  const boost::any& m_value;
  boost::python::object& m_result;
};

/// Conversion for basic types
struct PythonToAny
{
  PythonToAny(const boost::python::object& value, boost::any& result, bool& found) :
  m_value(value),
  m_result(result),
  m_found(found)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(m_found)
      return;

    //if(!PyObject_IsInstance(m_value.ptr(), at_key<T>(python_type_map)))
    if(m_value.ptr()->ob_type != at_key<T>(python_type_map))
      return;

    boost::python::extract<T> extracted_value(m_value);
    cf3_assert(extracted_value.check())
    m_found = true;
    m_result = extracted_value();
  }

  void operator()(const common::URI&) const
  {
    if(m_found)
      return;

    boost::python::extract< const common::URI& > extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_result = extracted_value();
    }
  }

  void operator()(const common::UUCount&) const
  {
    if(m_found)
      return;

    boost::python::extract< const common::UUCount& > extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_result = extracted_value();
    }
  }

  void operator()(const Handle<common::Component>&) const
  {
    if(m_found)
      return;

    boost::python::extract< ComponentWrapper& > extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_result = extracted_value().component().handle<common::Component>();
    }
  }

  const boost::python::object& m_value;
  boost::any& m_result;
  bool& m_found;
};

/// Conversion for lists
struct PythonListToAny
{
    PythonListToAny(const boost::python::list& a_list, boost::any& result, bool& found) :
    m_list(a_list),
    m_result(result),
    m_found(found)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(m_found)
      return;

    boost::any any_item;
    bool found_item = false;
    PythonToAny(m_list[0], any_item, found_item)(T());
    if(!found_item)
      return;

    const Uint list_len = boost::python::len(m_list);

    std::vector<T> result; result.reserve(list_len);
    result.push_back(boost::any_cast<T>(any_item));
    for(Uint i = 1; i != list_len; ++i)
    {
      found_item = false;
      PythonToAny(m_list[i], any_item, found_item)(T());
      cf3_assert(found_item);
      result.push_back(boost::any_cast<T>(any_item));
    }

    m_found = true;

    m_result = result;
  }

  const boost::python::list& m_list;
  boost::any& m_result;
  bool& m_found;
};

boost::python::api::object any_to_python(const boost::any& value)
{
  boost::python::object result;
  boost::mpl::for_each<AnyToPythonTypes>(AnyToPython(value, result));
  if(result.is_none()) // try lists if straight conversion failed
    boost::mpl::for_each<AnyToPythonTypes>(AnyListToPython(value, result));
  if(result.is_none())
    throw common::CastingFailed(FromHere(), "Failed to convert boost any to a valid python object");
  return result;
}

boost::any python_to_any(const boost::python::api::object& value)
{
  boost::any result;
  bool found = false;

  if(value.ptr()->ob_type == &PyList_Type) // object is a list
  {
    const boost::python::list& value_list = static_cast<const boost::python::list&>(value);
    if(boost::python::len(value_list) == 0)
      return boost::any();

    boost::mpl::for_each<PythonToAnyTypes>(PythonListToAny(value_list, result, found));
  }
  else
  {
    boost::mpl::for_each<PythonToAnyTypes>(PythonToAny(value, result, found));
  }

  if(!found)
    throw common::CastingFailed(FromHere(), std::string("Failed to convert to boost::any from python type ") + value.ptr()->ob_type->tp_name);

  return result;
}


} // python
} // cf3
