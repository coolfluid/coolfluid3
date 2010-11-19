// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionComponent_hpp
#define CF_Common_OptionComponent_hpp

/////////////////////////////////////////////////////////////////////////////////////

#include "Common/Option.hpp"
#include "Common/Component.hpp"
#include "Common/XmlHelpers.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  /// Class defines options to be used in the ConfigObject class
  /// @author Tiago Quintino
  template < typename BASETYPE >
  class OptionComponent : public Option
  {
  public:

    typedef std::string value_type;

    /// constructor
    OptionComponent ( const std::string& name, const std::string& desc, const std::string& def_name);

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @returns the xml tag for this option
    virtual const char * tag() const { return XmlParams::tag_node_map(); }

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return value<std::string>(); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return def<std::string>(); }

    //@} END VIRTUAL FUNCTIONS

  protected:

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

  protected:

    /// storage of the component pointer
    typename BASETYPE::Ptr m_component;

  }; // class OptionComponent

////////////////////////////////////////////////////////////////////////////////

  template < typename BASETYPE >
  OptionComponent<BASETYPE>::OptionComponent ( const std::string& name, const std::string& desc, const std::string& def_name) :
      Option(name, desc, def_name)
  {
    if (def_name!="")
    {
      Common::SafePtr< typename BASETYPE::PROVIDER > prov =
          Common::Factory<BASETYPE>::instance().get_provider( def_name );
      m_component = boost::dynamic_pointer_cast<BASETYPE>( prov->create(name) );
      m_value = m_component;
    }


  //  CFinfo
  //      << " creating OptionComponent [" << m_name << "]"
  //      << " of type [" << m_type << "]"
  //      << " w default [" << def_str() << "]"
  //      << " w desc [" << m_description << "]"
  //      << CFendl;
  }

  template < typename BASETYPE >
  void OptionComponent<BASETYPE>::configure ( XmlNode& node )
  {
    XmlParams params ( node );

    if ( params.option_map == 0 )
      throw  Common::XmlError( FromHere(), "ConfigObject received  XML without a \'" + std::string(XmlParams::tag_node_map()) + "\' node");

    std::string name  = params.get_option<std::string>("name");
    std::string atype = params.get_option<std::string>("atype");
    std::string ctype = params.get_option<std::string>("ctype");

    if ( atype != type() )
      throw BadValue ( FromHere(), "Option [" + m_name + "] received configuration with wrong abstract type [" + atype + "]" );

    m_value = ctype;

    m_component.reset();                       // delete previous pointee

    Common::SafePtr< typename BASETYPE::PROVIDER > prov =
        Common::Factory<BASETYPE>::instance().get_provider( ctype );
    m_component = boost::dynamic_pointer_cast<BASETYPE>( prov->create(name) );
    m_value = m_component;
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionComponent_hpp
