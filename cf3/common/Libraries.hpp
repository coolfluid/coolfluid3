// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Libraries_hpp
#define cf3_common_Libraries_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"

namespace cf3 {
namespace common {

class Library;

////////////////////////////////////////////////////////////////////////////////

/// Component that defines global environment
/// @author Quentin Gasper
class Common_API Libraries : public Component
{

public: // functions

  /// Contructor
  /// @param name of the component
  Libraries ( const std::string& name );

  /// Virtual destructor
  virtual ~Libraries();

  /// Get the class name
  static std::string type_name () { return "Libraries"; }

  /// Converts a CF3 library namespace to the library name.
  /// For example: cf3Common to coolfluid_common
  static std::string namespace_to_libname( const std::string& libnamespace );

  /// gives access to the factory of supplied type,
  /// insuring that in case it does not exist it gets built.
  template < typename LIB >
  Handle<LIB> library ()
  {
    const std::string lname = LIB::library_namespace(); //instead of LIB::type_name();
    Handle<Component> clib = get_child(lname);

    if ( is_null(clib) ) // doesnt exist so build it
    {
      cf3::common::TypeInfo::instance().regist< LIB >( lname );
      Handle<LIB> lib(create_component< LIB >(lname));
      cf3_assert( is_not_null(lib) );
      return lib;
    }

    // try to convert existing ptr to Handle<LIB> and return it
    Handle<LIB> lib(clib);

    if( is_null(lib) ) // conversion failed
      throw CastingFailed( FromHere(),
                          "Found component in Libraries with name "
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
  Handle<Library> autoload_library_with_builder( const std::string& builder_name );

  /// Attempts to load a CF3 plugin library from its namespace
  /// @param [in] namespace of the shared library to be loaded (e.g. cf3Common)
  /// @throws ValueNotFound in case of library not able to be loaded
  Handle<Library> autoload_library_with_namespace( const std::string& libnamespace );

  /// @name SIGNALS
  //@{

  /// Signal to load a list of libraries
  void signal_load_libraries ( SignalArgs& args );
  /// Signature of the signal to load a list of libraries
  void signature_load_libraries ( SignalArgs& args );

  /// Signal to autoload a list of libraries by namespace names
  void signal_load ( SignalArgs& args );
  /// Signature of the signal to autoload a list of libraries by namespace names
  void signature_load ( SignalArgs& args );

  //@} END SIGNALS

}; // Libraries

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Libraries_hpp
