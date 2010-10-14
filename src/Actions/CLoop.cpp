// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"
#include "Actions/CLoop.hpp"

#include "Mesh/CRegion.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Actions {

  using namespace Common;
  
/////////////////////////////////////////////////////////////////////////////////////

void CLoop::defineConfigProperties ( Common::PropertyList& options )
{
  std::vector< URI > dummy;
  options.add_option< OptionArrayT < URI > > ("Regions", "Regions to loop over", dummy)->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

CLoop::CLoop ( const CName& name ) :
  CAction(name)
{
  BUILD_COMPONENT;
  m_property_list["Regions"].as_option().attach_trigger ( boost::bind ( &CLoop::trigger_Regions,   this ) );
}

/////////////////////////////////////////////////////////////////////////////////////

CLoopOperation& CLoop::create_action(const std::string action_provider)
{
  // The execuation of operations must be in chronological order, hence
  // they get an alphabetical name
  std::string name = action_provider;
  CLoopOperation::Ptr sub_operation = 
    (create_component_abstract_type<CLoopOperation>(action_provider,name));
  add_component(sub_operation);
  return *sub_operation;
}

/////////////////////////////////////////////////////////////////////////////////////

void CLoop::trigger_Regions()
{
  std::vector<URI> vec; property("Regions").put_value(vec);
  BOOST_FOREACH(const CPath region_path, vec)
  {
    m_loop_regions.push_back(look_component_type<CRegion>(region_path));
  }
}

/////////////////////////////////////////////////////////////////////////////////////

const CLoopOperation& CLoop::action(const CName& name) const
{
  return *get_child_type<CLoopOperation const>(name);
}

/////////////////////////////////////////////////////////////////////////////////////

CLoopOperation& CLoop::action(const CName& name)
{
  return *get_child_type<CLoopOperation>(name);
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////