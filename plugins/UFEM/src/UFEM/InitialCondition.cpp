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

#include "InitialCondition.hpp"
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

common::ComponentBuilder < InitialCondition, common::Action, LibUFEM > InitialCondition_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialCondition::InitialCondition(const std::string& name) : solver::Action(name)
{
  options().add("field_tag", "")
    .pretty_name("Field Tag")
    .description("Tag for the field in which the initial conditions will be set")
    .attach_trigger(boost::bind(&InitialCondition::trigger_tag, this));
}

InitialCondition::~InitialCondition()
{
}


void InitialCondition::trigger_tag()
{
  const std::string field_tag = options().option("field_tag").value<std::string>();
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
      options().add(var_name, 0.).description(std::string("Initial condition for variable " + var_name));
    }
    else
    {
      options().add(var_name, std::vector<Real>()).description(std::string("Initial condition for variable " + var_name));
    }
  }
}

void InitialCondition::execute()
{
  if(m_loop_regions.empty())
    CFwarn << "No regions to loop over for action " << uri().string() << CFendl;
  
  const std::string field_tag = options().option("field_tag").value<std::string>();
  VariablesDescriptor& descriptor = find_component_with_tag<VariablesDescriptor>(physical_model().variable_manager(), field_tag);

  // Construct a vector with the values to use
  std::vector<Real> values; values.reserve(descriptor.size());
  const Uint nb_vars = m_variable_options.size();
  CFdebug << "Using descriptor " << descriptor.description() << " for InitialCondition" << CFendl;
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
      cf3_assert(val.size() == descriptor.var_length(i));
      values.insert(values.end(), val.begin(), val.end());
    }
  }
  BOOST_FOREACH(const Handle<Region>& region, m_loop_regions)
  {
    CFdebug << "  Action " << name() << ": running over region " << region->uri().path() << CFendl;
    std::vector<Uint> nodes;
    make_node_list(*region, common::find_parent_component<mesh::Mesh>(*region).geometry_fields().coordinates(), nodes);
    Field& field = find_field(*region, field_tag);
    BOOST_FOREACH(const Uint node, nodes)
    {
      field.set_row(node, values);
    }
  }
}



} // UFEM
} // cf3
