// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"
#include "common/Log.hpp"
#include "common/Foreach.hpp"
#include "common/Option.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/TimedComponent.hpp"
#include "common/TypeInfo.hpp"
#include "common/Signal.hpp"
#include "common/UUCount.hpp"

#include "common/XML/FileOperations.hpp"

#include "python/ComponentWrapper.hpp"
#include "python/PythonAny.hpp"
#include "python/TableWrapper.hpp"

namespace cf3 {
namespace python {

/// Available types for options
typedef boost::mpl::vector8<std::string, int, Uint, bool, Real, common::URI, Handle<common::Component>, common::UUCount > OptionTypes;

struct OptionCreator
{
  OptionCreator(common::OptionList& options, const boost::python::object& value, const std::string& name, bool& found) :
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

    boost::python::extract<T> extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_options.add(m_name, extracted_value());
    }
  }

  void operator()(const common::URI) const
  {
    if(m_found)
      return;

    boost::python::extract<common::URI> extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_options.add(m_name, extracted_value());
    }
  }

  common::OptionList& m_options;
  const boost::python::object& m_value;
  const std::string& m_name;
  bool& m_found;
};

// Wrapper for signals
struct SignalWrapper
{
  SignalWrapper(common::SignalPtr signal, const Handle<common::Component>& component) :
    m_signal(signal),
    m_component(component)
  {
  }

  boost::python::object operator()(boost::python::tuple args, boost::python::dict kwargs)
  {
    // Get the signature
    common::SignalArgs node;
    ( * m_signal->signature() ) ( node );
    common::XML::SignalOptions options(node);

    // We support keywordless calling only for signals taking a single mandatory argument
    // This is because the options are in alphabetical order and thus different from C++, making it counter-intuitive to learn
    // two different orders. This policy can be reconsidered when the options store is no longer alphabetic.
    if(len(args) == 2)
    {
      // We have only one (possibly optional) argument, so just set it
      if(options.store.size() == 1)
      {
        options.begin()->second->change_value(python_to_any(args[1]));
      }
      else
      {
        // Find the first non-optional argument and set it
        bool found = false;
        for(common::OptionList::iterator opt_it = options.begin(); opt_it != options.end(); ++opt_it)
        {
          if(opt_it->second->has_tag("basic"))
          {
            if(found)
              throw common::IllegalCall(FromHere(), "Method " + m_signal->name() + " can not be called using unnamed arguments since it has more than one basic option");
            opt_it->second->change_value(python_to_any(args[1]));
            found = true;
          }
        }
      }
    }
    else // All other cases should use keywords
    {
      const boost::python::list keys = kwargs.keys();
      const Uint nb_kwargs = len(keys);
      for(Uint i = 0; i != nb_kwargs; ++i)
      {
        boost::python::extract<std::string> extracted_key(keys[i]);
        const std::string key = extracted_key();
        if(options.check(key))
        {
          common::Option& option = options[key];
          option.change_value(python_to_any(kwargs[key]));
        }
        else
        {
          bool found = false;
          boost::mpl::for_each<OptionTypes>(OptionCreator(options, kwargs[key], key, found));
          if(!found)
            throw common::BadValue(FromHere(), "No conversion found for keyword argument " + key);
        }
      }

    }

    options.flush();

    CFdebug << "Calling signal " << m_signal->name() << " with arguments: ";
    for(SignalOptions::const_iterator opt = options.begin(); opt != options.end(); ++opt)
      CFdebug << opt->first << " = " << opt->second->value_str() << ", ";
    CFdebug << CFendl;

    (*m_signal->signal())(node);
    // Process reply
    SignalFrame reply = node.get_reply();
    if(reply.node.is_valid())
    {
      SignalOptions reply_options(reply);
      if(reply_options.check("created_component"))
        return wrap_component(m_component->access_component(reply_options["created_component"].value< common::URI >()));
      if(reply_options.check("return_value"))
        return any_to_python(reply_options["return_value"].value());
    }

    return boost::python::object();
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

  void bind_function(boost::python::object& python_object)
  {
    boost::python::object signal_func = boost::python::raw_function(*this, 0);
    setattr(signal_func, "__doc__", documentation());
    setattr(python_object, m_signal->name(), boost::python::import("types").attr("MethodType")(signal_func, python_object));
  }

  common::SignalPtr m_signal;
  Handle<common::Component> m_component;
};

struct ComponentWrapper::Implementation
{
  Implementation(const Handle<common::Component>& component) :
    m_component(component),
    m_list_interface(0)
  {
  }

  ~Implementation()
  {
    if(is_not_null(m_list_interface))
      delete m_list_interface;
  }

  Handle<common::Component> m_component;
  std::vector<SignalWrapper> m_wrapped_signals;
  PythonListInterface* m_list_interface;
};

ComponentWrapper::ComponentWrapper(const Handle< common::Component >& component) :
  m_implementation(new Implementation(component))
{
  // Add the signals
  boost_foreach(common::SignalPtr signal, component->signal_list())
  {
    if(signal->name() != "create_component")
      wrap_signal(signal);
  }
}

ComponentWrapper::~ComponentWrapper()
{
}

common::Component& ComponentWrapper::component()
{
  if(is_null(m_implementation->m_component))
    throw common::BadPointer(FromHere(), "Wrapped object was deleted");

  return *m_implementation->m_component;
}

void ComponentWrapper::bind_signals(boost::python::object& python_object)
{
  boost_foreach(SignalWrapper& signal, m_implementation->m_wrapped_signals)
  {
    signal.bind_function(python_object);
  }
}

void ComponentWrapper::wrap_signal(common::SignalPtr signal)
{
  // CFdebug << "Wrapping signal " << signal->name() << CFendl;
  m_implementation->m_wrapped_signals.push_back(SignalWrapper(signal, m_implementation->m_component));
}

boost::python::object wrap_component(const Handle<common::Component>& component)
{
  if(is_null(component))
    return boost::python::object();

  boost::python::object result = boost::python::object(ComponentWrapper(component));
  ComponentWrapper& wrapped = boost::python::extract<ComponentWrapper&>(result);
  wrapped.bind_signals(result);

  // Add extra functionality for derved classes
  add_ctable_methods(wrapped, result);

  return result;
}

void ComponentWrapper::set_list_interface(PythonListInterface* interface)
{
  if(is_not_null(m_implementation->m_list_interface))
    delete m_implementation->m_list_interface;

  m_implementation->m_list_interface = interface;
}


PythonListInterface* ComponentWrapper::get_list_interface()
{
  return m_implementation->m_list_interface;
}


std::string name(ComponentWrapper& self)
{
  return self.component().name();
}

boost::python::object create_component(ComponentWrapper& self, const std::string& name, const std::string& builder_name)
{
  boost::shared_ptr< common::Component > built_comp = common::build_component(builder_name, name);
  self.component().add_component(built_comp);
  return wrap_component(built_comp->handle<common::Component>());
}

boost::python::object get_child(ComponentWrapper& self, const std::string& name)
{
  return wrap_component(self.component().get_child(name));
}

boost::python::object access_component(ComponentWrapper& self, const std::string& uri)
{
  return wrap_component(self.component().access_component(uri));
}

common::URI uri(ComponentWrapper& self)
{
  return self.component().uri();
}

std::string derived_type_name(ComponentWrapper& self)
{
  return self.component().derived_type_name();
}

void print_timing_tree(ComponentWrapper& self)
{
  cf3::common::print_timing_tree(self.component());
}

void configure_option_recursively(ComponentWrapper& self, const std::string& option_name, const boost::python::object& value)
{
    self.component().configure_option_recursively(option_name, python_to_any(value));
}

Uint get_len(ComponentWrapper& self)
{
  if(is_null(self.get_list_interface()))
    throw common::NotSupported(FromHere(), "Object does not support len()");

  return self.get_list_interface()->len();
}

boost::python::object get_item(ComponentWrapper& self, const Uint i)
{
  if(is_null(self.get_list_interface()))
    throw common::NotSupported(FromHere(), "Object does not support indexing");

  return self.get_list_interface()->get_item(i);
}

void set_item(ComponentWrapper& self, const Uint i, boost::python::object& value)
{
  if(is_null(self.get_list_interface()))
    throw common::NotSupported(FromHere(), "Object does not support indexing");

  return self.get_list_interface()->set_item(i, value);
}

std::string to_str(ComponentWrapper& self)
{
  if(is_null(self.get_list_interface()))
  {
    std::stringstream output_str;
    output_str << "Component of type " << self.component().derived_type_name() << " at URI " << self.component().uri().path();
    return output_str.str();
  }

  return self.get_list_interface()->to_str();
}

bool is_equal(ComponentWrapper& self, ComponentWrapper& other)
{
  return self.component().handle() == other.component().handle();
}

bool is_not_equal(ComponentWrapper& self, ComponentWrapper& other)
{
  return self.component().handle() != other.component().handle();
}

// class OptionListWrapper
// {
// public:
//   OptionListWrapper(common::OptionList& option_list) : m_option_list(option_list)
//   {
//   }
//
//   OptionList& wrapped()
//   {
//     return m_option_list;
//   }
//
// private:
//
//   common::OptionList* m_option_list;
// };


//////////////////// OptionList ///////////////////////////////////////////////////////////////

common::OptionList* options(ComponentWrapper& self)
{
  return &self.component().options();
}

common::OptionList* set(common::OptionList* self, const std::string& optname, const boost::python::object& val)
{
  cf3_assert(is_not_null(self));
  common::Option& option = self->option(optname);
  option.change_value(python_to_any(val));
  return self;
}

std::string option_value_str(const common::OptionList* self, const std::string& optname)
{
  cf3_assert(is_not_null(self));
  return self->option(optname).value_str();
}

boost::python::object option_get_item(const common::OptionList* self, const std::string& optname){
  return any_to_python(self->operator [](optname).value());
}

// Function for __set_item__ must not return anything
void option_set_item(common::OptionList* self, const std::string& optname, const boost::python::object& val)
{
  set(self, optname, val);
}

boost::python::list option_keys(const common::OptionList* self){
  boost::python::list list;
  common::OptionList::const_iterator it=self->begin();
  for (;it!=self->end();it++){
    list.append(boost::python::str(it->first.c_str()));
  }
  return list;
}


boost::python::dict option_dict(const common::OptionList* self){
  boost::python::dict dict;
  common::OptionList::const_iterator it=self->begin();
  for (;it!=self->end();it++){
    dict[boost::python::str(it->first.c_str())]=any_to_python(self->operator [](it->first).value());
  }
  return dict;
}

//////////////////// PropertyList /////////////////////////////////////////////////////////////

common::PropertyList* properties(ComponentWrapper& self)
{
  return &self.component().properties();
}

Uint properties_get_len(common::PropertyList* self)
{
  return self->store.size();
}

boost::python::object properties_get_item(common::PropertyList* self, const std::string& name)
{
  return any_to_python(self->property(name));
}

void properties_set_item(common::PropertyList* self, const std::string& name, const boost::python::object& val)
{
  if (self->check(name)){
    self->property(name)=python_to_any(val);
  }
}

void properties_add(common::PropertyList* self, const std::string& name, const std::string& type, const boost::python::object& val)
{
  self->add(name, python_to_any(val));
}

boost::python::list properties_keys(const common::PropertyList* self){
  boost::python::list list;
  common::PropertyList::const_iterator it=self->begin();
  for (;it!=self->end();it++){
    list.append(boost::python::str(it->first.c_str()));
  }
  return list;
}


boost::python::dict properties_dict(const common::PropertyList* self){
  boost::python::dict dict;
  common::PropertyList::const_iterator it=self->begin();
  for (;it!=self->end();it++){
    dict[boost::python::str(it->first.c_str())]=any_to_python(self->property(it->first));
  }
  return dict;
}

///////////////////////////////////////////////////////////////////////////////////////////////

boost::python::str uucount_str(const common::UUCount* self){
  return boost::python::str(self->string().c_str());
}

boost::python::list uucount_uuid(const common::UUCount* self){
  boost::python::list list;
  const boost::uuids::uuid& uuid=self->uuid();
  for (int i=0,e=uuid.static_size();i<e;i++){
    list.append(uuid.data[i]);
  }
  return list;
}

Uint uucount_count(const common::UUCount* self){
  return self->count();
}

///////////////////////////////////////////////////////////////////////////////////////////////

void def_component()
{
  boost::python::class_<ComponentWrapper>("Component", boost::python::no_init)
    .def("name", name, "The name of this component")
    .def("create_component", create_component, "Create a new component, named after the first argument and built using the builder name in the second argument")
    .def("get_child", get_child)
    .def("access_component", access_component)
    .def("print_timing_tree", print_timing_tree)
    .def("options", options, boost::python::return_value_policy<boost::python::reference_existing_object>())
    .def("properties", properties, boost::python::return_value_policy<boost::python::reference_existing_object>())
    .def("uri", uri)
    .def("derived_type_name", derived_type_name, "Derived type name, i.e. the type of the concrete component")
    .def("configure_option_recursively", configure_option_recursively, "Configure the given option recursively")
    .def("__len__", get_len)
    .def("__getitem__", get_item)
    .def("__setitem__", set_item)
    .def("__str__", to_str)
    .def("__repr__", to_str)
    .def("__eq__", is_equal)
    .def("__ne__", is_not_equal);

  boost::python::class_<common::OptionList>("OptionList", boost::python::no_init)
    .def("set", set, boost::python::return_value_policy<boost::python::reference_existing_object>())
    .def("as_str", option_value_str, "String value for an option")
    .def("__getitem__", option_get_item, "")
    .def("__setitem__", option_set_item, "")
    .def("keys", option_keys, "")
    .def("dict", option_dict, "");

  boost::python::class_<common::PropertyList>("PropertyList", boost::python::no_init)
    .def("__len__", properties_get_len)
    .def("__getitem__", properties_get_item)
    .def("__setitem__", properties_set_item, "")
    .def("add", properties_add,"")
    .def("keys", properties_keys, "")
    .def("dict", properties_dict, "");

  boost::python::class_<common::UUCount>("UUCount", boost::python::no_init)
    .def("__str__", uucount_str)
    .def("__repr__", uucount_str)
    .def("uuid", uucount_uuid)
    .def("count", uucount_count);
}

} // python
} // cf3
