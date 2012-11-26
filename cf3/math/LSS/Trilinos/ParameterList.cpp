// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "Teuchos_ConfigDefs.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

#include "ParameterList.hpp"

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Option.hpp"
#include "common/Signal.hpp"
#include <common/EventHandler.hpp>


namespace cf3 {
namespace math {
namespace LSS {

namespace detail
{

  /// Convert a Trilinos parameter list name to a valid component name. We just remove the spaces now, keeping a CamelCase name.
  std::string param_list_name_to_comp(std::string param_list_name)
  {
    boost::replace_all(param_list_name, " ", "");
    return param_list_name;
  }

  /// Convert a parameter name to an option name (i.e. lowercase it and replace spaces with underscores)
  std::string param_name_to_option(std::string param_name)
  {
    boost::to_lower(param_name);
    boost::replace_all(param_name, ":", "");
    boost::replace_all(param_name, " ", "_");
    boost::replace_all(param_name, "-", "_");
    return param_name;
  }

  struct AddParameter
  {
    AddParameter(ParameterList& parameters, const Teuchos::ParameterEntry& entry, const std::string& name, bool& found) :
      m_parameters(parameters),
      m_entry(entry),
      m_name(name),
      m_found(found)
    {
    }

    template<typename T>
    void operator()(const T&)
    {
      if(m_entry.isType<T>())
      {
        cf3_assert(!m_found);
        m_parameters.options().add(param_name_to_option(m_name), Teuchos::getValue<T>(m_entry))
          .pretty_name(m_name)
          .attach_trigger(boost::bind(&ParameterList::trigger_parameter_changed, &m_parameters))
          .mark_basic();

        m_found = true;
      }
    }

    ParameterList& m_parameters;
    const Teuchos::ParameterEntry& m_entry;
    const std::string& m_name;
    bool& m_found;
  };

  struct UpdateParameterValue
  {
    UpdateParameterValue(const common::Option& option, Teuchos::ParameterEntry& entry, bool& found) :
      m_option(option),
      m_entry(entry),
      m_found(found)
    {
    }

    template<typename T>
    void operator()(const T&)
    {
      if(m_entry.isType<T>())
      {
        cf3_assert(!m_found);
        m_entry.setValue(m_option.value<T>());
        m_found = true;
      }
    }

    const common::Option& m_option;
    Teuchos::ParameterEntry& m_entry;
    bool& m_found;
  };
  
  struct AddNewParameter
  {
    AddNewParameter(ParameterList& parameters, Teuchos::ParameterList& teuchos_parameters, const std::string& name, const boost::any& value, bool& found) :
      m_parameters(parameters),
      m_teuchos_parameters(teuchos_parameters),
      m_name(name),
      m_value(value),
      m_found(found)
    {
    }

    template<typename T>
    void operator()(const T&)
    {
      if(m_value.type() == typeid(T))
      {
        const T typed_val = boost::any_cast<T>(m_value);
        cf3_assert(!m_found);
        m_parameters.options().add(param_name_to_option(m_name), typed_val)
          .pretty_name(m_name)
          .attach_trigger(boost::bind(&ParameterList::trigger_parameter_changed, &m_parameters))
          .mark_basic();
          
        m_teuchos_parameters.set(m_name, typed_val);

        m_found = true;
      }
    }

    ParameterList& m_parameters;
    Teuchos::ParameterList& m_teuchos_parameters;
    const std::string& m_name;
    const boost::any& m_value;
    bool& m_found;
  };

} // namespace detail

common::ComponentBuilder<ParameterList, common::Component, LibLSS> ParameterList_builder;

typedef boost::mpl::vector4<int, double, std::string, bool> ParameterTypesT;

ParameterList::ParameterList(const std::string& name): Component(name)
{
  regist_signal("add_parameter")
    .connect(boost::bind( &ParameterList::signal_add_parameter, this, _1 ))
    .description("Add a new parameter to the underlying list.")
    .pretty_name("Add Parameter");
    
  regist_signal( "create_parameter_list" )
    .connect( boost::bind( &ParameterList::signal_create_parameter_list, this, _1 ) )
    .description("Create a new ParameterList")
    .pretty_name("Create ParameterList")
    .signature( boost::bind ( &ParameterList::signature_create_parameter_list, this, _1) );
}

ParameterList::~ParameterList()
{
}

void ParameterList::set_parameter_list(Teuchos::ParameterList& parameters)
{
  m_parameters = Teuchos::rcpFromRef(parameters);
  
  for(Teuchos::ParameterList::ConstIterator it = parameters.begin(); it != parameters.end(); ++it)
  {
    if(it->second.isList())
    {
      Handle<ParameterList> new_parameters = create_component<ParameterList>(detail::param_list_name_to_comp(it->first));
      new_parameters->mark_basic();
      new_parameters->set_parameter_list(Teuchos::getValue<Teuchos::ParameterList>(it->second));
    }
    else
    {
      bool found = false;
      boost::mpl::for_each<ParameterTypesT>(detail::AddParameter(*this, it->second, it->first, found));
      if(!found)
      {
        CFdebug << "ParameterList: skipping parameter " << it->first << ": unsupported type " << it->second.getAny().typeName() << CFendl;
      }
    }
  }
}

Handle< ParameterList > ParameterList::create_parameter_list ( const std::string& trilinos_name )
{
  Handle<ParameterList> new_list = create_component<ParameterList>(detail::param_list_name_to_comp(trilinos_name));
  new_list->set_parameter_list(m_parameters->sublist(trilinos_name));
  return new_list;
}

void ParameterList::trigger_parameter_changed()
{
  for(common::OptionList::const_iterator it = options().begin(); it != options().end(); ++it)
  {
    bool found = false;
    boost::mpl::for_each<ParameterTypesT>(detail::UpdateParameterValue(*it->second, m_parameters->getEntry(it->second->pretty_name()), found));
    cf3_assert(found);
  }

  common::XML::SignalOptions options;
  options.add("parameters_uri", uri());

  common::SignalArgs f = options.create_frame();
  common::Core::instance().event_handler().raise_event( "trilinos_parameters_changed", f );
}

void ParameterList::signal_add_parameter(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  const std::string name = args.options().option("name").value<std::string>();
  const boost::any val = args.options().option("value").value();

  bool found = false;
  boost::mpl::for_each<ParameterTypesT>(detail::AddNewParameter(*this, *m_parameters, name, val, found));
  
  common::XML::SignalOptions event_options;
  event_options.add("parameters_uri", uri());
  common::SignalArgs f = event_options.create_frame();
  common::Core::instance().event_handler().raise_event( "trilinos_parameters_changed", f );
}

void ParameterList::signal_create_parameter_list ( common::SignalArgs& args )
{
  common::XML::SignalOptions options(args);
  Handle<ParameterList> new_list = create_parameter_list(options.option("trilinos_name").value<std::string>());
  
  common::XML::SignalFrame reply = args.create_reply(uri());
  common::XML::SignalOptions reply_options(reply);
  reply_options.add("created_component", new_list->uri());
}

void ParameterList::signature_create_parameter_list ( common::SignalArgs& args )
{
  common::XML::SignalOptions options(args);
  options.add("trilinos_name", "unknown")
    .pretty_name("Trilinos Name")
    .description("Name of the new parameter list, as required by Trilinos");
}



} // namespace LSS
} // namespace math
} // namespace cf3
