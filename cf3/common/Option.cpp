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

void Option::configure_option ( XmlNode& node )
{
  cf3_assert ( node.is_valid() );
  change_value(extract_configured_value(node));
}

////////////////////////////////////////////////////////////////////////////

Option& Option::attach_trigger ( TriggerT trigger )
{
  m_triggers.push_back(trigger);
  return *this;
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
  m_value = value; // update the value
  copy_to_linked_params(m_linked_params);

  // call all trigger functors
  trigger();
};

////////////////////////////////////////////////////////////////////////////////

void Option::trigger () const
{
  // call all trigger functors
  boost_foreach( const Option::TriggerT& call_trigger, m_triggers )
    call_trigger();
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
