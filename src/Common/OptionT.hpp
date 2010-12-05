// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionT_hpp
#define CF_Common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "Common/Option.hpp"

#include "Common/BasicExceptions.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines one option to be used by the ConfigObject class
  /// This class supports the following types:
  ///   - bool
  ///   - int
  ///   - std::string
  ///   - boost::filesystem::path
  ///   - CF::Uint
  ///   - CF::Real
  ///   - CF::Common::URI
  /// @author Tiago Quintino
  template < typename TYPE >
      class Common_API OptionT : public Option  {

  public:

    typedef TYPE value_type;

    OptionT ( const std::string& name, const std::string& desc, value_type def);

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @returns the xml tag for this option
    virtual const char * tag() const;

    /// @returns the value as a sd::string
    virtual std::string value_str () const { return from_value ( value<TYPE>() ); }

    /// @returns the default value as a sd::string
    virtual std::string def_str () const  { return from_value ( def<TYPE>() ); }

    //@} END VIRTUAL FUNCTIONS

  protected: // functions

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

    /// copy the configured update value to all linked parameters
    virtual void copy_to_linked_params ( const boost::any& val );

  }; // class OptionT

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionT_hpp
