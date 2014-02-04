// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"

#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "Proto/Expression.hpp"

#include "CopyScalar.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CopyScalar, common::Action, LibActions > CopyScalar_Builder;

///////////////////////////////////////////////////////////////////////////////////////

using namespace Proto;

CopyScalar::CopyScalar ( const std::string& name ) :
  ProtoAction(name)
{
  options().add("source_field_tag", "navier_stokes_p_solution")
    .pretty_name("Source Field Tag")
    .description("Tag for the source field")
    .attach_trigger(boost::bind(&CopyScalar::trigger_options, this))
    .mark_basic();
    
  options().add("source_variable_name", "Pressure")
    .pretty_name("Source Variable Name")
    .description("Variable name for the source tag")
    .attach_trigger(boost::bind(&CopyScalar::trigger_options, this))
    .mark_basic();
    
  options().add("target_field_tag", "")
    .pretty_name("Target Field Tag")
    .description("Tag for the target field")
    .attach_trigger(boost::bind(&CopyScalar::trigger_options, this))
    .mark_basic();
    
  options().add("target_variable_name", "")
    .pretty_name("Target Variable Name")
    .description("Variable name for the target tag")
    .attach_trigger(boost::bind(&CopyScalar::trigger_options, this))
    .mark_basic();
    
  trigger_options();
}

void CopyScalar::trigger_options()
{
  const std::string source_field_tag = options().value<std::string>("source_field_tag");
  const std::string source_variable_name = options().value<std::string>("source_variable_name");
  std::string target_field_tag = options().value<std::string>("target_field_tag");
  std::string target_variable_name = options().value<std::string>("target_variable_name");
  
  if(target_field_tag.empty())
    target_field_tag = "copy_" + source_field_tag;
  
  if(target_variable_name.empty())
    target_variable_name = "copy_" + source_variable_name;
  
  FieldVariable<0, ScalarField> source(source_variable_name, source_field_tag);
  FieldVariable<1, ScalarField> target(target_variable_name, target_field_tag);
  
  set_expression(nodes_expression(target = source));
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

