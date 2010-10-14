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

  class OptionURI : public Option
  {

  public:

    typedef URI value_type;
    typedef boost::shared_ptr<OptionURI> Ptr;
    typedef boost::shared_ptr<OptionURI const> ConstPtr;

    OptionURI(const std::string & name, const std::string & desc,
              const URI & def);

    virtual ~OptionURI();

    void supported_protocol(const std::string & protocol);

    std::vector<std::string> supported_protocols() const { return m_protocols; }

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @returns the xml tag for this option
    virtual const char * tag() const { return XmlTag<URI>::type(); }

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return from_value( value<URI>() ); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return from_value( def<URI>() ); }

    //@} END VIRTUAL FUNCTIONS

  protected: // functions

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

  private:

    std::vector<std::string> m_protocols;

  }; // class OptionURI

  /////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

///////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionURI_hpp
