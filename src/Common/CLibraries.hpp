// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CLibraries_hpp
#define CF_Common_CLibraries_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component that defines global environment
  /// @author Quentin Gasper
  class Common_API CLibraries : public Component {

  public: //typedefs

    typedef boost::shared_ptr<CLibraries> Ptr;
    typedef boost::shared_ptr<CLibraries const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CLibraries ( const std::string& name );

    /// Virtual destructor
    virtual ~CLibraries();

    /// Get the class name
    static std::string type_name () { return "CLibraries"; }

    /// gives access to the factory of supplied type,
    /// insuring that in case it does not exist it gets built.
    template < typename LIB >
    typename LIB::Ptr get_library ()
    {
      const std::string lname = LIB::library_namespace(); //instead of LIB::type_name();
      typename LIB::Ptr lib = get_child<LIB>(lname);
      if ( lib == nullptr )
      {
        CF::TypeInfo::instance().regist< LIB >( lname );
        lib = create_component< LIB >(lname);
      }
      cf_assert( lib != nullptr );
      return lib;
    }

    /// @name SIGNALS
    //@{

    /// creates a component from this component
    void load_library ( XmlNode& node );

    void load_library_signature ( XmlNode& node );

    //@} END SIGNALS


}; // CLibraries

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CLibraries_hpp
