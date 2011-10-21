// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "common/CFactory.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

CFactory::CFactory(const std::string& name): Component(name)
{
}

CFactory::~CFactory()
{
}

/// @brief Checks the builder reduced name matches
/// The reduced name is the name without the namespace preceding it
class IsBuilderReducedName {
private:
  std::string m_builder_reduced_name;
public:
  IsBuilderReducedName () : m_builder_reduced_name() {}
  IsBuilderReducedName (const std::string& name) : m_builder_reduced_name(name) {}

  bool operator()(Component::ConstPtr component) const
  {
//    std::cout << "checking [" << Builder::extract_reduced_name( component->name() )
//              << "] == [" << m_builder_reduced_name << std::endl;

    return ( Builder::extract_reduced_name( component->name() ) == m_builder_reduced_name );
  }

  bool operator()(const Component& component) const
  {
//    std::cout << "checking [" << Builder::extract_reduced_name( component.name() )
//              << "] == [" << m_builder_reduced_name << std::endl;

    return ( Builder::extract_reduced_name( component.name() ) == m_builder_reduced_name );
  }
};


Component& CFactory::find_builder_with_reduced_name(const std::string& name)
{
  IsBuilderReducedName filter ( name );

  std::vector<Component::Ptr> found;
  boost_foreach( Component& comp, find_components_with_filter( (*this), filter ) )
  {
    Builder::Ptr builder = comp.as_ptr<Builder>();
      if( is_not_null(builder) )
        found.push_back( comp.self() );
  }

  if ( found.empty() )
    throw ValueNotFound( FromHere(), "Builder with name \'" + name +
                                     "\' not found in factory \'" + uri().string() + "\'" );

  if ( found.size() > 1 )
  {
    std::ostringstream msg;

    msg << "Multiple builders in factory \'" << uri().string()
        << "\' match the reduced name \'" + name + "\'. Matching builders:";

    boost_foreach(Component::Ptr comp, found)
        msg << "\n  -  " << comp->name();

    msg << "\nTry accessing the builder using its full name.";

    throw BadValue( FromHere(), msg.str() );
  }

  return *(found[0]);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
