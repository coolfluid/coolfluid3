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
    boost::replace_all(param_name, " ", "_");
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

} // namespace detail

common::ComponentBuilder<ParameterList, common::Component, LibLSS> ParameterList_builder;

typedef boost::mpl::vector4<int, double, std::string, bool> ParameterTypesT;

ParameterList::ParameterList(const std::string& name): Component(name)
{
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


} // namespace LSS
} // namespace math
} // namespace cf3
