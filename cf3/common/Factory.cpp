// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <sstream>

#include "common/Factory.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

/////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

Factory::Factory(const std::string& name): Component(name)
{
}

Factory::~Factory()
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

  bool operator()(const Handle<Component const>& component) const
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


Builder& Factory::find_builder_with_reduced_name(const std::string& name)
{
  return find_component_with_filter<Builder>(*this, IsBuilderReducedName(name));
}

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
