// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"

#include "Common/PropertyList.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Property::Ptr PropertyList::add_property (const std::string& name,
                                            const boost::any & value )
  {
    cf_assert_desc ( "Class has already property with same name",
                     this->m_properties.find(name) == m_properties.end() );
    Property::Ptr prop ( new Property(value) );
    m_properties.insert( std::make_pair(name, prop ) );
    return prop;
  }

  const Property & PropertyList::getProperty( const std::string& pname) const
  {
    PropertyStorage_t::const_iterator itr = m_properties.find(pname);
    if ( itr != m_properties.end() )
      return *itr->second.get();
    else
		{ 			
			std::string msg;
			msg += "Property with name ["+pname+"] not found. Available properties are:\n";
			PropertyStorage_t::const_iterator it = m_properties.begin();
			for (; it!=m_properties.end(); it++)
				msg += "  - " + it->first + "\n";
			throw ValueNotFound(FromHere(),msg);
		}
		
	}

  const Option & PropertyList::getOption( const std::string& pname) const
  {
    return getProperty(pname).as_option();
  }

  void PropertyList::erase( const std::string& pname)
  {
    PropertyStorage_t::iterator itr = m_properties.find(pname);
    if ( itr != m_properties.end() )
      m_properties.erase(itr);
    else
      throw ValueNotFound(FromHere(), "Property with name [" + pname + "] not found" );
  }

  Property & PropertyList::operator [] (const std::string & pname)
  {
    Property::Ptr prop;
    PropertyStorage_t::iterator itr = m_properties.find(pname);

    if ( itr != m_properties.end() )
      prop = itr->second;
    else
      prop = add_property(pname, boost::any());

    return *prop.get();
  }

	void PropertyList::configure_property(const std::string& pname, const boost::any& val)
	{
		PropertyStorage_t::iterator itr = m_properties.find(pname);
		if (itr == m_properties.end())
		{
			std::string msg;
			msg += "Property with name ["+pname+"] not found. Available properties are:\n";
			PropertyStorage_t::iterator it = m_properties.begin();
			for (; it!=m_properties.end(); it++)
				msg += "  - " + it->first + "\n";
			throw ValueNotFound(FromHere(),msg);
		}
		
		Property::Ptr prop = itr->second;
		
		// update the value and trigger its actions (if it is an option)
		if(prop->is_option())
			prop->as_option().change_value(val);
		else
			prop->change_value(val);
	}
	

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

