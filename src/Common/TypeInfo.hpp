// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_TypeInfo_hpp
#define CF_Common_TypeInfo_hpp

#include <typeinfo>

#include "Common/Assertions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// Function to demangle the return of typeid()
Common_API std::string demangle (const char* type);

#define CF_DEMANGLE_TYPEID(a) CF::Common::demangle(typeid(a).name())

/// @brief Handles type information
/// This struct allows to associate a type to a string. It is a singleton.
/// @author Tiago Quintino
/// @author Quentin Gasper
struct Common_API TypeInfo : public boost::noncopyable
{

  /// @brief Constructor.
  /// Registers all supported otion types.
  TypeInfo();

  /// @return Returns the instance
  static TypeInfo& instance();

  /// @brief Registers a type to a string
  /// @param tname The type name
  template <typename TYPE>
  void  regist ( const std::string& tname )
  {
    portable_types[typeid(TYPE).name()] = tname ;
  }

  /// @brief Map for types and their associated string
  /// The key is the type, the value is the associated string. The type
  /// is converted to a string using @c typeid
  std::map< std::string, std::string> portable_types;

};

template < typename TYPE >
std::string class_name ()
{
  TypeInfo& ti = TypeInfo::instance();
  std::map<std::string, std::string>::const_iterator it =
      ti.portable_types.find(typeid(TYPE).name());

  cf_assert( it != ti.portable_types.end() );

  return it->second;
}

std::string class_name_from_typeinfo (const std::type_info & info);

// /// @brief Helper class to force TypeInfo registration
// /// @author Tiago Quintino
// template< typename TYPE >
// struct RegistTypeInfo
// {
//   /// @brief Registers this type into the TypeInfo registry
//   RegistTypeInfo( const std::string& name = TYPE::type_name() ) { TypeInfo::instance().regist<TYPE>(name); }
// };

template< typename TYPE, typename LIB>
struct RegistTypeInfo
{
  /// @brief Registers this type into the TypeInfo registry
  RegistTypeInfo( const std::string& name = LIB::library_namespace()+"."+TYPE::type_name() ) { TypeInfo::instance().regist<TYPE>(name); }
};

/// @brief Helper function to regist a type in the TypeInfo registry
/// Typically used within the constructor of a template class
/// @author Tiago Quintino
template< typename TYPE >
void regist_typeinfo( TYPE* self )
{
  TypeInfo::instance().regist<TYPE>( TYPE::type_name() );
}

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_TypeInfo_hpp
