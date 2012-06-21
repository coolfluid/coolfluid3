// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Tags.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"

#include "InitialConditionConstant.hpp"
#include "ParsedFunctionExpression.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace common::XML;
using namespace math;
using namespace mesh;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitialConditionConstant, common::Action, LibUFEM > InitialConditionConstant_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditionConstant::InitialConditionConstant(const std::string& name) : solver::Action(name)
{
  options().add("field_tag", "")
    .pretty_name("Field Tag")
    .description("Tag for the field in which the initial conditions will be set")
    .attach_trigger(boost::bind(&InitialConditionConstant::trigger_tag, this));
}

InitialConditionConstant::~InitialConditionConstant()
{
}


void InitialConditionConstant::trigger_tag()
{
  const std::string field_tag = options().value<std::string>("field_tag");
  if(field_tag.empty())
    return;

  // Wipe the old options
  BOOST_FOREACH(const std::string& opt_name, m_variable_options)
  {
    options().erase(opt_name);
  }

  m_variable_options.clear();

  VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physical_model().variable_manager(), field_tag);
  const Uint nb_vars = descriptor.nb_vars();
  for(Uint i = 0; i != nb_vars; ++i)
  {
    const std::string& var_name = descriptor.user_variable_name(i);
    m_variable_options.push_back(var_name);
    if(descriptor.var_length(i) == 1)
    {
      options().add(var_name, 0.).description(std::string("Initial condition for variable " + var_name)).mark_basic();
    }
    else
    {
      options().add(var_name, std::vector<Real>()).description(std::string("Initial condition for variable " + var_name)).mark_basic();
    }
  }
}

void InitialConditionConstant::execute()
{
  if(m_loop_regions.empty())
    CFwarn << "No regions to loop over for action " << uri().string() << CFendl;

  const std::string field_tag = options().value<std::string>("field_tag");
  VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physical_model().variable_manager(), field_tag);

  // Construct a vector with the values to use
  std::vector<Real> values; values.reserve(descriptor.size());
  const Uint nb_vars = m_variable_options.size();
  CFdebug << "Using descriptor " << descriptor.description() << " for InitialConditionConstant" << CFendl;
  cf3_assert(nb_vars == descriptor.nb_vars());

  for(Uint i = 0; i != nb_vars; ++i)
  {
    const Option& opt = options().option(m_variable_options[i]);
    if(descriptor.var_length(i) == 1) // scalar
    {
      values.push_back(opt.value<Real>());
    }
    else
    {
      std::vector<Real> val = opt.value< std::vector<Real> >();
      // By default, use zero
      if(val.empty())
        val.resize(descriptor.var_length(i), 0.);
      cf3_assert(val.size() == descriptor.var_length(i));
      values.insert(values.end(), val.begin(), val.end());
    }
  }
  BOOST_FOREACH(const Handle<Region>& region, m_loop_regions)
  {
    CFdebug << "  Action " << name() << ": running over region " << region->uri().path() << CFendl;
    
    // Build a list of used entities
    mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*region);
    std::vector< Handle<mesh::Entities const> > used_entities;
    BOOST_FOREACH(const mesh::Entities& entities, common::find_components_recursively<mesh::Entities>(*region))
    {
      used_entities.push_back(entities.handle<mesh::Entities>());
    }
    boost::shared_ptr< common::List<Uint> > used_nodes_ptr = mesh::build_used_nodes_list(used_entities, mesh.geometry_fields(), true);

    common::List<Uint>& nodes = *used_nodes_ptr;
    
    Field& field = find_field(*region, field_tag);
    BOOST_FOREACH(const Uint node, nodes.array())
    {
      field.set_row(node, values);
    }
  }
}



} // UFEM
} // cf3
