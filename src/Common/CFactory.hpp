// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_CFactory_hpp
#define cf3_common_CFactory_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a factory which builds other components
/// @author Tiago Quintino
class Common_API CFactory : public Component
{
public:

  typedef boost::shared_ptr<CFactory> Ptr;
  typedef boost::shared_ptr<CFactory const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CFactory(const std::string& name);

  /// @brief Virtual destructor.
  virtual ~CFactory();

  /// @returns the class name
  static std::string type_name() { return "CFactory"; }

  /// @return the name of the type of this factory
  virtual std::string factory_type_name() const = 0;

  /// returns a build by its name stripped out of the namespace part
  Component& find_builder_with_reduced_name( const std::string& name );

}; // CFactory

/////////////////////////////////////////////////////////////////////////////////

/// @brief Component class for a factory which builds other components
/// @author Tiago Quintino
template < typename TYPE >
class CFactoryT : public CFactory {
public:

  typedef boost::shared_ptr< CFactoryT<TYPE> > Ptr;
  typedef boost::shared_ptr< CFactoryT<TYPE> const> ConstPtr;

  /// @brief Contructor
  /// @param name of component
  CFactoryT(const std::string& name) : CFactory(name) {}

  /// @brief Virtual destructor.
  virtual ~CFactoryT() {}

  /// @returns the class name
  static std::string type_name() { return "CFactoryT<" + TYPE::type_name() + ">"; }

  /// @return the name of the type of this factory
  virtual std::string factory_type_name() const { return TYPE::type_name(); }

}; // CFactoryT

/////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

/////////////////////////////////////////////////////////////////////////////////

#endif // CF3_common_CFactory_hpp

