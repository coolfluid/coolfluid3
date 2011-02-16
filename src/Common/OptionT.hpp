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

    /// @brief Checks whether the option has a list of restricted values.
    /// @return Returns @c true if the option a such list; otherwise, returns
    /// @c false.
    bool has_restricted_list() const { return m_restricted_list.size() > 1; }

    virtual std::string data_type() const { return type(); }

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

    //@} END VIRTUAL FUNCTIONS

  protected: // functions

    /// copy the configured update value to all linked parameters
    virtual void copy_to_linked_params ( const boost::any& val );

  }; // class OptionT

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionT_hpp
