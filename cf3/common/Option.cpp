// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>

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
  // cf3_assert_desc("The name of option ["+name+"] does not comply with coolfluid standard. "
  //                "It may not contain spaces.",
  //   boost::algorithm::all(name,
  //     boost::algorithm::is_alnum() || boost::algorithm::is_any_of("-_")) );
}

////////////////////////////////////////////////////////////////////////////////

//Option::Option(const std::string& name, const std::string& pretty_name, const std::string& desc, boost::any def)
//  : m_value(def),
//    m_default(def),
//    m_name(name),
//    m_pretty_name(pretty_name),
//    m_description(desc)
//{
//  // cf3_assert_desc("The name of option ["+name+"] does not comply with coolfluid standard. "
//  //                "It may not contain spaces.",
//  //   boost::algorithm::all(name,
//  //     boost::algorithm::is_alnum() || boost::algorithm::is_any_of("-_")) );
//}

////////////////////////////////////////////////////////////////////////////

Option::~Option()
{

}

////////////////////////////////////////////////////////////////////////////

void Option::configure_option ( XmlNode& node )
{
  cf3_assert ( node.is_valid() );

  this->configure(node); // update the value

  // call all trigger functors
  trigger();
}

////////////////////////////////////////////////////////////////////////////

Option& Option::operator =( const boost::any & new_value )
{
  change_value( new_value );
  return *this;
}

////////////////////////////////////////////////////////////////////////////

Option::Ptr Option::mark_basic()
{
  if(!has_tag("basic"))
    add_tag("basic");
  return shared_from_this();
}

////////////////////////////////////////////////////////////////////////////

void Option::change_value ( const boost::any& value )
{
  //  cf3_assert(/*m_restricted_list.size() == 1 ||*/
  //            std::find(m_restricted_list.begin(), m_restricted_list.end(), value)
  //            != m_restricted_list.end());

  boost::any data = value_to_data(value);
  m_value = data; // update the value
  copy_to_linked_params(data);
    // call all trigger functors
  trigger();
};

////////////////////////////////////////////////////////////////////////////////

void Option::trigger () const
{
  // call all trigger functors
  boost_foreach( const Option::Trigger_t& call_trigger, m_triggers )
    call_trigger();
}

////////////////////////////////////////////////////////////////////////////////

std::string Option::type() const
{
  return class_name_from_typeinfo(m_value.type());
}

//////////////////////////////////////////////////////////////////////////////

Option::Ptr Option::description ( const std::string & description )
{
  m_description = description;
  return shared_from_this();
}

//////////////////////////////////////////////////////////////////////////////

Option::Ptr Option::pretty_name ( const std::string & pretty_name )
{
  m_pretty_name = pretty_name;
  return shared_from_this();
}

//////////////////////////////////////////////////////////////////////////////

Option::Ptr Option::separator ( const std::string & separator )
{
  m_separator = separator;
  return shared_from_this();
}

//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
