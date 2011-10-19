// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CLibraries_hpp
#define cf3_common_CLibraries_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace cf3 {
namespace common {

class CLibrary;

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

    /// Converts a CF3 library namespace to the library name.
    /// For example: CF.Common to coolfluid_common
    static std::string namespace_to_libname( const std::string& libnamespace );

    /// gives access to the factory of supplied type,
    /// insuring that in case it does not exist it gets built.
    template < typename LIB >
    typename LIB::Ptr library ()
    {
      const std::string lname = LIB::library_namespace(); //instead of LIB::type_name();
      Component::Ptr clib = get_child_ptr(lname);

      typename LIB::Ptr lib;
      if ( is_null(clib) ) // doesnt exist so build it
      {
        cf3::common::TypeInfo::instance().regist< LIB >( lname );
        lib = create_component_ptr< LIB >(lname);
        cf_assert( is_not_null(lib) );
        return lib;
      }

      // try to convert existing ptr to LIB::Ptr and return it
      lib = clib->as_ptr<LIB>();

      if( is_null(lib) ) // conversion failed
        throw CastingFailed( FromHere(),
                            "Found component in CLibraries with name "
                            + lname
                            + " but is not the actual library "
                            + LIB::type_name() );
      return lib;
    }

    /// calls all the initiate hooks on all the libraries that have not been initiated yet
    void initiate_all_libraries();
    /// calls all the terminate hooks on all the libraries that have not been terminated yet
    void terminate_all_libraries();

    /// Checks if a CF3 plugin library is already loaded
    /// @param [in] file URI to the shared library to be loaded
    bool is_loaded(  const std::string& name );

    /// Attempts to load a CF3 plugin library
    /// @param [in] file URI to the shared library to be loaded
    void load_library( const URI& file );

    /// Attempts to load a CF3 plugin library from the builders name
    /// @param [in] name of the shared library to be loaded
    /// @throws ValueNotFound in case of library not able to be loaded
    boost::shared_ptr<CLibrary> autoload_library_with_builder( const std::string& builder_name );

    /// Attempts to load a CF3 plugin library from its namespace
    /// @param [in] namespace of the shared library to be loaded (e.g. CF.Common)
    /// @throws ValueNotFound in case of library not able to be loaded
    boost::shared_ptr<CLibrary> autoload_library_with_namespace( const std::string& libnamespace );

    /// @name SIGNALS
    //@{

    /// Signal to load a list of libraries
    void signal_load_libraries ( SignalArgs& args );
    /// Signature of the signal to load a list of libraries
    void signature_load_libraries ( SignalArgs& args );

    //@} END SIGNALS

}; // CLibraries

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_common_CLibraries_hpp
