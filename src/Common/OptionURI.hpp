// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionURI_hpp
#define CF_Common_OptionURI_hpp

///////////////////////////////////////////////////////////////////////////////

#include "Common/URI.hpp"

#include "Common/Option.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  /////////////////////////////////////////////////////////////////////////////

  class Common_API OptionURI : public Option
  {

  public:

    typedef URI value_type;
    typedef boost::shared_ptr<OptionURI> Ptr;
    typedef boost::shared_ptr<OptionURI const> ConstPtr;

    OptionURI(const std::string & name, const std::string & desc,
              const URI & def);

    static Option::Ptr create(const std::string & name, const std::string & desc, const URI& def)
    {
      return Option::Ptr ( new OptionURI(name,desc,def) );
    }
    
    static Option::Ptr create(const std::string & name, const std::string & desc, const URI& def, URI::Scheme::Type protocol)
    {
      Option::Ptr option ( new OptionURI(name,desc,def) );
      boost::dynamic_pointer_cast<OptionURI>(option)->supported_protocol(protocol);
      return option;
    }

    virtual ~OptionURI();

    void supported_protocol(URI::Scheme::Type protocol);

    std::vector<URI::Scheme::Type> supported_protocols() const { return m_protocols; }

    /// @name VIRTUAL FUNCTIONS
    //@{
      
    virtual std::string data_type() const { return type(); }

    /// @returns the xml tag for this option
    virtual const char * tag() const { return XmlTag<URI>::type(); }

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return from_value( value<URI>() ); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return from_value( def<URI>() ); }

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

    //@} END VIRTUAL FUNCTIONS

  protected: // functions

    /// copy the configured update value to all linked parameters
    virtual void copy_to_linked_params ( const boost::any& val );

  private:

    std::vector<URI::Scheme::Type> m_protocols;

  }; // class OptionURI

  /////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionURI_hpp
