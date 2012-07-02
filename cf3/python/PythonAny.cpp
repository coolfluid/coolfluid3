// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "BoostPython.hpp"
#include "PythonAny.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

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
typedef boost::mpl::vector9<std::string, Uint, int, bool, Real, common::URI, common::UUCount, Handle<common::Component>, Handle<common::Component const> > AnyToPythonTypes;

/// Extract the type string for the elements of a python list
/// Only arrays with the same overall type are supported. Arrays containing a single
/// floating point value are assumed to contain only floating point values
std::string python_list_element_type(const boost::python::list& pylist)
{
  const Uint nb_elems = boost::python::len(pylist);
  std::string result;
  for(Uint i = 0; i != nb_elems; ++i)
  {
    std::string elem_type = type_name(pylist[i]);
    if(boost::starts_with(elem_type, "handle["))
      elem_type = common::class_name< Handle<common::Component> >();
    if((result == common::class_name<int>() && elem_type == common::class_name<Real>()) || result.empty())
    {
      // First assign or upgrade from integer
      result = elem_type;
    }
    else
    {
      // Other elements must match
      if(!(elem_type == result || (elem_type == common::class_name<int>() && result == common::class_name<Real>())))
      {
        throw common::BadValue(FromHere(), "Python list element " + boost::lexical_cast<std::string>(i) + " does not match expected list type " + result);
      }
    }
  }
  if(result.empty())
    throw common::BadValue(FromHere(), "Unable to determine element type for empty python list");

  return result;
}

/// Conversion from any to python for basic types
struct AnyToPython
{
  AnyToPython(const boost::any& value, boost::python::object& result, bool& found) :
  m_value(value),
  m_result(result),
  m_found(found)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(m_found || typeid(T) != m_value.type())
    {
      return;
    }

    m_found = true;

    m_result = boost::python::object(boost::any_cast<T>(m_value));
  }

  void operator()(const Handle<common::Component>&) const
  {
    if(m_found || typeid(Handle<common::Component>) != m_value.type())
    {
      return;
    }

    m_found = true;

    m_result = boost::python::object( wrap_component(boost::any_cast< Handle<common::Component> >(m_value)));
  }

  // TODO: const-correctness for python? This would require a second componentwrapper, for const components...
  void operator()(const Handle<common::Component const>&) const
  {
    if(m_found || typeid(Handle<common::Component const>) != m_value.type())
    {
      return;
    }

    m_found = true;

    common::Component* comp = const_cast<common::Component*>(boost::any_cast< Handle<common::Component const> >(m_value).get());
    m_result = boost::python::object(wrap_component(is_null(comp) ? Handle<common::Component>() : comp->handle()));
  }

  const boost::any& m_value;
  boost::python::object& m_result;
  bool& m_found;
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
      bool found = false;
      AnyToPython(item, list_item, found)(item);
      cf3_assert(found);
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

    if(type_name(m_value) != common::class_name<T>())
      return;

    boost::python::extract<T> extracted_value(m_value);
    cf3_assert(extracted_value.check())
    m_found = true;
    m_result = extracted_value();
  }

  void operator()(const Real&) const
  {
    if(m_found)
      return;

    if(type_name(m_value) != common::class_name<Real>() && type_name(m_value) != common::class_name<int>())
      return;

    boost::python::extract<Real> extracted_value(m_value);
    cf3_assert(extracted_value.check())
    m_found = true;
    m_result = extracted_value();
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

    if(python_list_element_type(m_list) != common::class_name<T>())
      return;

    const Uint list_len = boost::python::len(m_list);
    std::vector<T> result; result.reserve(list_len);
    for(Uint i = 0; i != list_len; ++i)
    {
      boost::any any_item;
      bool found_item = false;
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
  bool found = false;
  boost::mpl::for_each<AnyToPythonTypes>(AnyToPython(value, result, found));
  if(!found) // try lists if straight conversion failed
    boost::mpl::for_each<AnyToPythonTypes>(AnyListToPython(value, result));
  if(!found && result.is_none())
    throw common::CastingFailed(FromHere(), std::string("Failed to convert boost any of type ") + value.type().name() + " to a valid python object");
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

std::string type_name(const boost::python::api::object& python_object)
{
  // Mapping between the python type name and the coolfluid type
  typedef std::map<std::string, std::string> TypeMapT;
  static const TypeMapT python_type_map =
    boost::assign::map_list_of(std::string(PyBool_Type.tp_name), common::class_name<bool>())
                              (std::string(PyInt_Type.tp_name), common::class_name<int>())
                              (std::string(PyString_Type.tp_name), common::class_name<std::string>())
                              (std::string(PyFloat_Type.tp_name), common::class_name<Real>())
                              (std::string(PyList_Type.tp_name), "array") // Special indication of lists
                              (std::string("URI"), common::class_name<common::URI>()) // "URI" as passed in the boost::python::class_ definition
                              (std::string("UUCount"), common::class_name<common::UUCount>()) // Same as URI
                              (std::string("Component"), "component"); // Same as URI, but needs special treatment afterwards

  // Look up the type in the map
  const PyTypeObject& python_type = *python_object.ptr()->ob_type;
  const TypeMapT::const_iterator it = python_type_map.find(python_type.tp_name);
  if(it == python_type_map.end())
  {
    throw common::ValueNotFound(FromHere(), std::string("Python type ") + python_type.tp_name + " has no coolfluid equivalent");
  }

  // Treat special cases
  if(it->second == "array")
  {
    const std::string element_type = python_list_element_type(static_cast<const boost::python::list&>(python_object));
    return "array[" + element_type + "]";
  }
  else if(it->second == "component")
  {
    boost::python::extract< ComponentWrapper& > extracted_value(python_object);
    cf3_assert(extracted_value.check());
    return "handle[" + extracted_value().component().derived_type_name() + "]";
  }

  return  it->second;
}


} // python
} // cf3
