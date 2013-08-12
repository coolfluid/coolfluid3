// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <set>

#include "common/Builder.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

#include "WriteRestartManager.hpp"


namespace cf3 {
namespace UFEM {

common::ComponentBuilder < WriteRestartManager, solver::actions::TimeSeriesWriter, LibUFEM > WriteRestartManager_Builder;

WriteRestartManager::WriteRestartManager ( const std::string& name ) : TimeSeriesWriter ( name )
{
  m_write_restart = create_static_component<solver::actions::WriteRestartFile>("Writer");
  m_write_restart->mark_basic();
  
  options().add("mesh", Handle<mesh::Mesh>())
    .pretty_name("Mesh")
    .description("Mesh from which to pick fields")
    .attach_trigger(boost::bind(&WriteRestartManager::trigger_setup, this));
    
  options().add("field_tags", std::vector<std::string>())
    .pretty_name("Field tags")
    .description("Tags to use when looking up fields")
    .attach_trigger(boost::bind(&WriteRestartManager::trigger_setup, this));
}


WriteRestartManager::~WriteRestartManager()
{
}

void WriteRestartManager::trigger_setup()
{
  Handle<mesh::Mesh> mesh = options().value< Handle<mesh::Mesh> >("mesh");
  if(is_null(mesh))
    return;
  
  const std::vector<std::string> field_tags = options().value< std::vector<std::string> >("field_tags");
  std::vector< Handle<mesh::Field> > fields; fields.reserve(field_tags.size());
  std::set<std::string> unique_tags;
  BOOST_FOREACH(const std::string& tag, field_tags)
  {
    if(!unique_tags.insert(tag).second) // Make sure we ignore any duplicate tags
      continue;
    
    Handle<mesh::Field> field = common::find_component_ptr_recursively_with_tag<mesh::Field>(*mesh, tag);
    if(is_null(field))
      throw common::SetupError(FromHere(), "Field with tag " + tag + " was not found for restarting");
    fields.push_back(field);
  }
  
  m_write_restart->options().set("fields", fields);
}


} // UFEM
} // cf3
