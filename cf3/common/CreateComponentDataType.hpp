// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_CreateComponentDataType_hpp
#define cf3_common_CreateComponentDataType_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

////////////////////////////////////////////////////////////////////////////////

/// @def create_component_data_type
/// @brief macro to create data wrapping component type
/// This macro creates a component class that wraps any given data type that has
/// a default constructor.
/// The data can be accessed directly through its memberfunction data()
/// @note This cannot be placed inside a function
/// @param data_type        The data type to wrap in the component class
/// @param API              The API of the library this class is created in
/// @param CName            The typename of the component class
/// @param Cname_type_name  The name of the class as a std::string
#define create_component_data_type( data_type , API , CName , Cname_type_name) \
class API CName : public common::Component                                     \
{                                                                              \
                                                                               \
public:                                                                        \
                                                                               \
  CName ( const std::string& name ) : common::Component(name)                  \
  {                                                                            \
    regist_typeinfo(this);                                                     \
  }                                                                            \
                                                                               \
  virtual ~CName() {}                                                          \
                                                                               \
  static std::string type_name () { return Cname_type_name; }                  \
                                                                               \
  const data_type & data() const { return m_data; }                            \
                                                                               \
  data_type & data() { return m_data; }                                        \
                                                                               \
private:                                                                       \
                                                                               \
  data_type m_data;                                                            \
                                                                               \
};

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_CreateComponentDataType_hpp
