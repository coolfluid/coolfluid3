// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/function.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Foreach.hpp"
#include "common/Option.hpp"
#include "common/XML/XmlNode.hpp"

using namespace boost::assign;
using namespace cf3::common::XML;

namespace cf3 {
namespace common {

Option::Option(const std::string & name, boost::any def)
  : m_value(def),
    m_default(def),
    m_name(name),
    m_pretty_name(),
    m_description(),
    m_separator(";")
{
}

////////////////////////////////////////////////////////////////////////////////

Option::~Option()
{
}

////////////////////////////////////////////////////////////////////////////

void Option::set ( XmlNode& node )
{
  cf3_assert ( node.is_valid() );
  change_value(extract_configured_value(node));
}

////////////////////////////////////////////////////////////////////////////

Option& Option::attach_trigger ( TriggerT trigger )
{
  attach_trigger_tracked(trigger);
  return *this;
}

////////////////////////////////////////////////////////////////////////////

Option::TriggerID Option::attach_trigger_tracked ( Option::TriggerT trigger )
{
  const TriggerID new_id = m_current_connection_id;
  m_triggers[m_current_connection_id++] = trigger;
  return new_id;
}


////////////////////////////////////////////////////////////////////////////

void Option::detach_trigger( const Option::TriggerID trigger_id )
{
  m_triggers.erase(trigger_id);
}


////////////////////////////////////////////////////////////////////////////

Option& Option::operator =( const boost::any & new_value )
{
  change_value( new_value );
  return *this;
}

////////////////////////////////////////////////////////////////////////////

Option& Option::mark_basic()
{
  if(!has_tag("basic"))
    add_tag("basic");
  return *this;
}

////////////////////////////////////////////////////////////////////////////

void Option::change_value ( const boost::any& value )
{
  change_value_impl(value);
  copy_to_linked_params(m_linked_params);

  // call all trigger functors
  trigger();
};

////////////////////////////////////////////////////////////////////////////////

void Option::trigger () const
{
  // Copy, so we protect against the trigger list being modified during execution
  Option::TriggerStorageT triggers = m_triggers;
  // call all trigger functors
  for(Option::TriggerStorageT::const_iterator trig_it = triggers.begin(); trig_it != triggers.end(); ++trig_it)
  {
    trig_it->second();
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string Option::type() const
{
  return class_name_from_typeinfo(m_value.type());
}

//////////////////////////////////////////////////////////////////////////////

std::string Option::element_type() const
{
  return type();
}

//////////////////////////////////////////////////////////////////////////////

Option& Option::description ( const std::string & description )
{
  m_description = description;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////

Option& Option::pretty_name ( const std::string & pretty_name )
{
  m_pretty_name = pretty_name;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////

Option& Option::separator ( const std::string & separator )
{
  m_separator = separator;
  return *this;
}

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
