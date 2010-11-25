// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CreateComponent_hpp
#define CF_Common_CreateComponent_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/CBuilder.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////

/// Create a (sub)component of a given abstract type specified type
/// @param provider_name the registry string of the provider of the concrete type
/// @name name to give to the created omponent
template < typename ATYPE >
    typename ATYPE::Ptr create_component_abstract_type ( const std::string& builder_name, const std::string& name )
{
  CFactories::Ptr factories = Core::instance().root()->get_child_type< CFactories >("Factories");

  CFactory::Ptr factory = factories->get_child_type< CFactory >( ATYPE::type_name() );

  CBuilder::Ptr builder = factory->get_child_type< CBuilder >( builder_name );

  cf_assert ( builder != nullptr ); /// @todo maybe this should be an exception

  Component::Ptr comp = builder->build ( name );

  cf_assert ( comp != nullptr ); /// @todo maybe this should be an exception

  return boost::dynamic_pointer_cast<ATYPE>( comp );
}

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_CreateComponent_hpp
