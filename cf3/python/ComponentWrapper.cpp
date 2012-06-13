// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"
#include "common/ComponentIterator.hpp"
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
#include <common/OptionFactory.hpp>
#include <common/EventHandler.hpp>

#include "python/ComponentWrapper.hpp"
#include "python/PythonAny.hpp"
#include "python/TableWrapper.hpp"
#include "python/Utility.hpp"

namespace cf3 {
namespace python {

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
          const boost::python::object& kw_val = kwargs[key];
          if(kw_val.ptr()->ob_type == &PyList_Type)
          {
            const boost::python::list& kw_list = static_cast<const boost::python::list&>(kw_val);
            const Uint nb_vals = boost::python::len(kw_list);
            std::vector<std::string> values_str(nb_vals);
            for(Uint i = 0; i != nb_vals; ++i)
            {
              values_str[i] = boost::python::extract<std::string>(boost::python::str(kw_list[i]))();
            }
            options.add(common::OptionFactory::instance().create_option(key, type_name(kw_val), values_str));
          }
          else
          {
            std::string tp_name = type_name(kw_val);
            /// We need to translate to the base component class, since we don't know what the actual type of the expected option is
            if(boost::starts_with(tp_name, "handle["))
              tp_name = "handle[cf3.common.Component]";
            options.add(common::OptionFactory::instance().create_option(key, tp_name, boost::python::extract<std::string>(boost::python::str(kw_val))()));
          }
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
    // weak_ref to avoid circular reference
    generic_setattr(python_object, m_signal->name().c_str(), boost::python::import("types").attr("MethodType")(signal_func, weak_ref(python_object)));
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

  virtual ~Implementation()
  {
    if(is_not_null(m_list_interface))
      delete m_list_interface;
  }

  void bind_signals(boost::python::object& python_object)
  {
    boost_foreach(SignalWrapper& signal, m_wrapped_signals)
    {
      signal.bind_function(python_object);
    }
  }

  void wrap_signal(common::SignalPtr signal)
  {
    m_wrapped_signals.push_back(SignalWrapper(signal, m_component));
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
      m_implementation->wrap_signal(signal);
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

boost::python::object wrap_component(const Handle<common::Component>& component)
{
  if(is_null(component))
    return boost::python::object();

  boost::python::object result = boost::python::object(ComponentWrapper(component));
  ComponentWrapper& wrapped = boost::python::extract<ComponentWrapper&>(result);
  wrapped.m_implementation->bind_signals(result);

  cf3_assert(result.ptr()->ob_refcnt == 1); // Make sure there are no circular references
  
  // Add extra functionality for derved classes
  add_ctable_methods(wrapped, result);

  return result;  
}

/// Override setattr to allow direct handling of basic options
void component_setattr(boost::python::object& self, const std::string attr, const boost::python::object& value)
{
  common::Component& comp = boost::python::extract<ComponentWrapper&>(self)().component();

  Handle<common::Component> child_comp = comp.get_child(attr);
  if(is_not_null(child_comp) && child_comp->has_tag("basic"))
    throw common::IllegalCall(FromHere(), "Assigning to a sub-component is not allowed");

  // Try setting an option, if there is a basic option with the attribute name
  std::string opt_name = attr;
  if(comp.options().check(opt_name) && comp.options().option(opt_name).has_tag("basic"))
  {
    comp.options().set(opt_name, python_to_any(value));
    return;
  }

  // Just set the attribute in all other cases
  generic_setattr(self, attr.c_str(), value);
}

/// Override for getattr to get basic options and sub-components
boost::python::object component_getattr(ComponentWrapper& self, const std::string& attr)
{
  common::Component& comp = self.component();
  if(comp.options().check( attr ) && comp.options().option( attr ).has_tag("basic"))
  {
    return any_to_python(comp.options().option(attr).value());
  }
  
  Handle<common::Component> child_comp = comp.get_child(attr);
  if(is_not_null(child_comp) && child_comp->has_tag("basic"))
    return wrap_component(child_comp);
  
  throw common::ValueNotFound(FromHere(), "Attribute " + attr + " does not exist as either a basic option or a basic component for object at " + comp.uri().path());
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
  return self.component().uri().string();
}

bool is_equal(ComponentWrapper& self, ComponentWrapper& other)
{
  return self.component().handle() == other.component().handle();
}

bool is_not_equal(ComponentWrapper& self, ComponentWrapper& other)
{
  return self.component().handle() != other.component().handle();
}

boost::python::object component_mark_basic(ComponentWrapper& self)
{
  self.component().mark_basic();
  return wrap_component(self.component().handle());
}

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
    .def("mark_basic", component_mark_basic, "Mark the component as basic")
    .def("__len__", get_len)
    .def("__getitem__", get_item)
    .def("__setitem__", set_item)
    .def("__str__", to_str)
    .def("__repr__", to_str)
    .def("__eq__", is_equal)
    .def("__ne__", is_not_equal)
    .def("__setattr__", component_setattr)
    .def("__getattr__", component_getattr);

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
