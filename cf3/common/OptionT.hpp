// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionT_hpp
#define cf3_common_OptionT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "common/Option.hpp"

#include "common/BasicExceptions.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines one option to be used by the ConfigObject class
  /// This class supports the following types:
  ///   - bool
  ///   - int
  ///   - std::string
  ///   - boost::filesystem::path
  ///   - cf3::Uint
  ///   - cf3::Real
  ///   - cf3::common::URI
  /// @author Tiago Quintino
  template < typename TYPE >
      class Common_API OptionT : public Option  {

  public:

    typedef TYPE value_type;

    OptionT ( const std::string& name, value_type def);

    static Option::Ptr create(const std::string & name, const TYPE& def)
    {
      return Option::Ptr ( new OptionT(name, def) );
    }

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @returns the xml tag for this option
    virtual const char * tag() const;

    /// @returns the value as a sd::string
    virtual std::string value_str () const;

    /// @returns the default value as a sd::string
    virtual std::string def_str () const;

    /// @brief Checks whether the option has a list of restricted values.
    /// @return Returns @c true if the option a such list; otherwise, returns
    /// @c false.
    bool has_restricted_list() const { return m_restricted_list.size() > 1; }

    virtual std::string data_type() const { return type(); }

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XML::XmlNode& node );

    //@} END VIRTUAL FUNCTIONS

  protected: // functions

    /// copy the configured update value to all linked parameters
    virtual void copy_to_linked_params ( const boost::any& val );

  }; // class OptionT

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionT_hpp
