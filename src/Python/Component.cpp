// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>

#include <boost/algorithm/string.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/Component.hpp"
#include "Common/Log.hpp"
#include "Common/Foreach.hpp"
#include "Common/Option.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/TimedComponent.hpp"
#include "Common/TypeInfo.hpp"
#include "Common/Signal.hpp"

#include "Common/XML/FileOperations.hpp"

#include "Math/MatrixTypes.hpp"

#include "Python/Component.hpp"

namespace cf3 {
namespace Python {

using namespace boost::python;

// Types that can be held by any
typedef boost::mpl::vector7<std::string, Real, Uint, int, bool, common::URI, RealVector> AnyTypes;

/// Conversion for basic types
struct PythonToAny
{
  PythonToAny(const object& value, boost::any& result, const std::string& target_type, bool& found) :
    m_value(value),
    m_result(result),
    m_target_type(target_type),
    m_found(found)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(m_found)
      return;

    if(common::class_name_from_typeinfo(typeid(T)) != m_target_type)
      return;

    extract<T> extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_result = extracted_value();
    }
  }

  const object& m_value;
  boost::any& m_result;
  const std::string& m_target_type;
  bool& m_found;
};

/// Conversion for lists
struct PythonListToAny
{
  PythonListToAny(const list& a_list, boost::any& result, const std::string& target_type, bool& found) :
    m_list(a_list),
    m_result(result),
    m_target_type(target_type),
    m_found(found)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(m_found)
      return;

    if(common::class_name_from_typeinfo(typeid(T)) != m_target_type)
      return;

    std::vector<T> vec;

    const Uint nb_items = len(m_list);
    vec.reserve(nb_items);
    for(Uint i = 0; i != nb_items; ++i)
    {
      extract<T> extracted_value(m_list[i]);
      if(!extracted_value.check())
        throw common::BadValue(FromHere(), "Incorrect python extracted value for list item");

      vec.push_back(extracted_value());
    }

    m_found = true;

    m_result = vec;
  }

  const list& m_list;
  boost::any& m_result;
  const std::string& m_target_type;
  bool& m_found;
};

struct OptionCreator
{
  OptionCreator(common::OptionList& options, const object& value, const std::string& name, bool& found) :
    m_options(options),
    m_value(value),
    m_name(name),
    m_found(found)
  {
  }

  template<typename T>
  void operator()(T) const
  {
    if(m_found)
      return;

    extract<T> extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_options.add_option< Common::OptionT<T> >(m_name, extracted_value());
    }
  }

  void operator()(const common::URI) const
  {
    if(m_found)
      return;

    extract<common::URI> extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_options.add_option< Common::OptionURI >(m_name, extracted_value());
    }
  }

  common::OptionList& m_options;
  const object& m_value;
  const std::string& m_name;
  bool& m_found;
};

// Helper functions to convert to any
boost::any python_to_any(const object& val, const std::string& target_type)
{
  boost::any result;
  bool found = false;

  const bool is_list = boost::starts_with(target_type, "array[");

  if(is_list)
  {
    const std::string single_value_type(target_type.begin()+6, target_type.end()-1);
    boost::mpl::for_each<AnyTypes>(PythonListToAny(static_cast<const list&>(val), result, single_value_type, found));
  }
  else
  {
    boost::mpl::for_each<AnyTypes>(PythonToAny(val, result, target_type, found));
  }

  if(!found)
    throw common::CastingFailed(FromHere(), "Failed to convert to boost::any while looking for target type " + target_type);

  return result;
}

// Wrapper for signals
struct SignalWrapper
{
  SignalWrapper(common::SignalPtr signal) :
    m_signal(signal)
  {
  }

  object operator()(tuple args, dict kwargs)
  {
    // Get the signature
    common::SignalArgs node;
    ( * m_signal->signature() ) ( node );
    common::XML::SignalOptions options(node);

    // We support keywordless calling only for signals taking a single argument
    const Uint nb_options = options.store.size();
    if(len(args) == 2 && nb_options == 1)
    {
      common::Option& option = *options.begin()->second;
      option.change_value(python_to_any(args[1], option.type()));
    }
    else // All other cases should use keywords
    {
      if(len(args) != 1)
        throw common::IllegalCall(FromHere(), "Method " + m_signal->name() + " can not be called using unnamed arguments");

//       for(Common::OptionList::iterator option_it = options.begin(); option_it != options.end(); ++option_it)
//       {
//         Common::Option& option = *option_it->second;
//         if(kwargs.has_key(option.name()))
//           option.change_value(python_to_any(kwargs[option.name()], option.type()));
//       }

      const list keys = kwargs.keys();
      const Uint nb_kwargs = len(keys);
      for(Uint i = 0; i != nb_kwargs; ++i)
      {
        extract<std::string> extracted_key(keys[i]);
        const std::string key = extracted_key();
        if(options.check(key))
        {
          common::Option& option = options[key];
          option.change_value(python_to_any(kwargs[key], option.type()));
        }
        else
        {
          bool found = false;
          boost::mpl::for_each<AnyTypes>(OptionCreator(options, kwargs[key], key, found));
          if(!found)
            throw common::BadValue(FromHere(), "No conversion found for keyword argument " + key);
        }
      }

    }

    options.flush();

    std::string node_contents;
    common::XML::to_string(node.node, node_contents);

    CFdebug << "Calling signal " << m_signal->name() << " with arguments:\n" << node_contents << CFendl;

    (*m_signal->signal())(node);

    return object();
  }

  std::string documentation()
  {
    std::stringstream doc_str;
    doc_str << m_signal->description() << "\n";
    doc_str << "Valid keywords are:\n";

    // Get the signature
    common::SignalArgs node;
    ( * m_signal->signature() ) ( node );

    common::XML::SignalOptions options(node);
    for(common::OptionList::iterator option_it = options.begin(); option_it != options.end(); ++option_it)
    {
      doc_str << "  " << option_it->first << ": " << option_it->second->description() << "\n";
    }

    return doc_str.str();
  }

  void bind_function(object& python_object)
  {
    object signal_func = raw_function(*this, 0);
    setattr(signal_func, "__doc__", documentation());
    setattr(python_object, m_signal->name(), import("types").attr("MethodType")(signal_func, python_object));
  }

  common::SignalPtr m_signal;
};

class ComponentWrapper
{
public:
  ComponentWrapper(common::Component& component) :
    m_component(component.as_ptr<Common::Component>())
  {
    // Add the signals
    bcommonoreach(Common::SignalPtr signal, component.signal_list())
    {
      if(signal->name() != "create_component")
        wrap_signal(signal);
    }
  }

  std::string name()
  {
    return component().name();
  }

  object create_component(const std::string& name, const std::string& builder_name)
  {
    common::Component::Ptr built_comp = common::build_component(builder_name, name);
    component().add_component(built_comp);
    return wrap_component(*built_comp);
  }

  object get_child(const std::string& name)
  {
    return wrap_component(component().get_child(name));
  }

  object access_component(const std::string& uri)
  {
    return wrap_component(component().access_component(uri));
  }

  void configure_option(const std::string& optname, const object& val)
  {
    common::Option& option = component().option(optname);
    option.change_value(python_to_any(val, option.type()));
  }

  std::string option_value_str(const std::string& optname)
  {
    return component().option(optname).value_str();
  }

  common::URI uri()
  {
    return component().uri();
  }

  void wrap_signal(common::SignalPtr signal)
  {
    m_wrapped_signals.push_back(SignalWrapper(signal));
  }

  void bind_signals(object& python_object)
  {
    boost_foreach(SignalWrapper& signal, m_wrapped_signals)
    {
      signal.bind_function(python_object);
    }
  }

  void print_timing_tree()
  {
    cf3::common::print_timing_tree(component());
  }

private:
  // checked access
  common::Component& component()
  {
    if(m_component.expired())
      throw common::BadPointer(FromHere(), "Wrapped object was deleted");

    return *m_component.lock();
  }
  boost::weak_ptr<common::Component> m_component;

  std::vector<SignalWrapper> m_wrapped_signals;
};

object wrap_component(common::Component& component)
{
  object result = object(ComponentWrapper(component));
  ComponentWrapper& wrapped = extract<ComponentWrapper&>(result);
  wrapped.bind_signals(result);
  return result;
}


void def_component()
{
  class_<ComponentWrapper>("Component", no_init)
    .def("name", &ComponentWrapper::name, "The name of this component")
    .def("create_component", &ComponentWrapper::create_component, "Create a new component, named after the first argument and built using the builder name in the second argument")
    .def("get_child", &ComponentWrapper::get_child)
    .def("access_component", &ComponentWrapper::access_component)
    .def("configure_option", &ComponentWrapper::configure_option)
    .def("option_value_str", &ComponentWrapper::option_value_str)
    .def("print_timing_tree", &ComponentWrapper::print_timing_tree)
    .def("uri", &ComponentWrapper::uri);
}

} // Python
} // cf3
