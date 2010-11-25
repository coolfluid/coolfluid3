// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CreateComponent_hpp
#define CF_Common_CreateComponent_hpp

/////////////////////////////////////////////////////////////////////////////////

#include "Common/CBuilder.hpp"
#include "Common/BasicExceptions.hpp"

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

  if ( !builder ) throw ValueNotFound( FromHere(), "CBuilder \'" + builder_name + "\' not found in factory \'" + ATYPE::type_name() + "\'" );

  Component::Ptr comp = builder->build ( name );

  if ( !comp ) throw NotEnoughMemory ( FromHere(), "CBuilder \'" + builder_name + "\' failed to allocate component with name \'" + name + "\'" );

  typename ATYPE::Ptr ccomp = boost::dynamic_pointer_cast<ATYPE>( comp );

  if ( !ccomp ) throw CastingFailed ( FromHere(), "Pointer created by CBuilder \'" + builder_name + "\' could not be casted to \'" + ATYPE::type_name() + "\' pointer" );

  return ccomp;
}

/////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

#endif // CF_Common_CreateComponent_hpp
