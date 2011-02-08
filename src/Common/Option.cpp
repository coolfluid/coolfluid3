// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/std/vector.hpp>

#include "Common/Option.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/Foreach.hpp"

using namespace boost::assign;
using namespace CF::Common;

Option::Option(const std::string & name, const std::string & desc, boost::any def)
  : Property(def),
    m_default(def),
    m_name(name),
    m_description(desc)
{
  m_is_option = true;
}

////////////////////////////////////////////////////////////////////////////

Option::~Option()
{

}

////////////////////////////////////////////////////////////////////////////

void Option::configure_option ( XmlNode& node )
{
  this->configure(node); // update the value

  // call all trigger functors
  trigger();
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
//  cf_assert(/*m_restricted_list.size() == 1 ||*/
//            std::find(m_restricted_list.begin(), m_restricted_list.end(), value)
//            != m_restricted_list.end());

  boost::any data = value_to_data(value);
  Property::change_value(data);
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

//////////////////////////////////////////////////////////////////////////////
