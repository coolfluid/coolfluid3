// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_Factory_hpp
#define cf3_common_Factory_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

class Builder;

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a factory which builds other components
/// @author Tiago Quintino
class Common_API Factory : public Component
{
public:

  /// @brief Contructor
  /// @param name of component
  Factory(const std::string& name);

  /// @brief Virtual destructor.
  virtual ~Factory();

  /// @returns the class name
  static std::string type_name() { return "Factory"; }

  /// @return the name of the type of this factory
  virtual std::string factory_type_name() const = 0;

  /// returns a build by its name stripped out of the namespace part
  Builder& find_builder_with_reduced_name( const std::string& name );

}; // Factory

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a factory which builds other components
/// @author Tiago Quintino
template < typename TYPE >
class FactoryT : public Factory {
public:

  /// @brief Contructor
  /// @param name of component
  FactoryT(const std::string& name) : Factory(name) {}

  /// @brief Virtual destructor.
  virtual ~FactoryT() {}

  /// @returns the class name
  static std::string type_name() { return "FactoryT<" + TYPE::type_name() + ">"; }

  /// @return the name of the type of this factory
  virtual std::string factory_type_name() const { return TYPE::type_name(); }

}; // FactoryT

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Factory_hpp

