// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_TypeInfo_hpp
#define cf3_common_TypeInfo_hpp

#include <typeinfo>

#include "common/Assertions.hpp"
#include "Handle.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/// Function to demangle the return of typeid()
Common_API std::string demangle (const char* type);

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

  cf3_assert_desc("type "+std::string(typeid(TYPE).name())+" not registered", it != ti.portable_types.end() );

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

/// Register type info for a class that belongs to a coolfluid library, including the full namespace in the name
/// Also registers type information for the handle and a vector of handles
template< typename TYPE, typename LIB>
struct RegistTypeInfo
{
  /// @brief Registers this type into the TypeInfo registry
  RegistTypeInfo( const std::string& name = LIB::library_namespace()+"."+TYPE::type_name() )
  {
    const std::string handle_name = "handle[" + name + "]";
    const std::string array_name = "array[" + handle_name + "]";
    
    TypeInfo::instance().regist<TYPE>(name);
    TypeInfo::instance().regist< Handle<TYPE> >(handle_name);
    TypeInfo::instance().regist< std::vector< Handle<TYPE> > >(array_name);
  }
};

/// @brief Helper function to regist a type in the TypeInfo registry
/// Typically used within the constructor of a template class
/// @author Tiago Quintino
template< typename TYPE >
void regist_typeinfo( TYPE* self )
{
  TypeInfo::instance().regist<TYPE>( TYPE::type_name() );
}

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_TypeInfo_hpp
