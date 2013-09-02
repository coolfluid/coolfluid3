// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/EventHandler.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Triag.hpp"

#include "BulkVelocity.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < BulkVelocity, common::Action, LibUFEM > BulkVelocity_Builder;

BulkVelocity::BulkVelocity(const std::string& name) :
  ProtoAction(name),
  m_changing_result(false)
{    
  options().add("history",m_history)
      .pretty_name("History")
      .description("History component used to log the history of the integral value.")
      .link_to(&m_history)
      .mark_basic();
      
  regist_signal ( "set_field" )
    .connect( boost::bind ( &BulkVelocity::signal_set_field, this, _1 ) )
    .description( "Set up the model using a specific solver" )
    .pretty_name( "Setup" )
    .signature( boost::bind ( &BulkVelocity::signature_set_field, this, _1 ) );
  
  options().add("result", 0.)
    .pretty_name("Result")
    .description("Result of the last bulk velocity computation")
    .mark_basic()
    .attach_trigger(boost::bind(&BulkVelocity::trigger_result, this));
}

BulkVelocity::~BulkVelocity()
{
}

void BulkVelocity::set_field(const std::string& variable_name, const std::string& tag)
{
  using boost::proto::lit;

  typedef boost::mpl::vector6<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Quad3D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP0::Line, mesh::LagrangeP0::Quad, mesh::LagrangeP0::Triag> element_types;
  
  // Indicate is an element is a owned by the current process
  FieldVariable<0, VectorField> v(variable_name, tag); // The velocity
  FieldVariable<1, ScalarField> local("is_local_element", "surface_integrator", mesh::LagrangeP0::LibLagrangeP0::library_namespace());
  
  set_expression(elements_expression(element_types(), group(
    lit(m_integral_value) += local*integral<1>(v * normal)[0],
    lit(m_area) += local*integral<1>(_norm(normal))
  )));
}

void BulkVelocity::signal_set_field(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  set_field(options.value<std::string>("variable_name"), options.value<std::string>("field_tag"));
}

void BulkVelocity::signature_set_field(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  
  options.add("variable_name", "SomeScalar")
    .pretty_name("Variable Name")
    .description("Name of the variable to use");

  options.add("field_tag", "SomeFieldTag")
    .pretty_name("Field Tag")
    .description("Tag for the field to use");
}


void BulkVelocity::execute()
{
  m_integral_value = 0.;
  m_area = 0.;
  
  if(m_loop_regions.empty())
  {
    CFwarn << "BulkVelocity has no regions" << CFendl;
    return;
  }

  mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(*m_loop_regions.front());

  const std::string elems_name = "cf3.mesh.LagrangeP0";
  Handle<mesh::Dictionary> elems_P0_handle(mesh.get_child(elems_name));
  mesh::Dictionary& elems_P0 = is_null(elems_P0_handle) ? mesh.create_discontinuous_space(elems_name,"cf3.mesh.LagrangeP0") : *elems_P0_handle;
  if(!elems_P0.has_tag("ufem_dict"))
    elems_P0.add_tag("ufem_dict");

  Handle<mesh::Field> is_local_element(elems_P0.get_child("surface_integrator"));
  if(is_null(is_local_element))
  {
    is_local_element = elems_P0.create_field("surface_integrator", "is_local_element").handle<mesh::Field>();
    is_local_element->add_tag("surface_integrator");
  }
  if(!is_local_element->has_tag("local_element_initialized"))
  {
    const Uint nb_elems = is_local_element->size();
    for(Uint i = 0; i != nb_elems; ++i)
    {
      is_local_element->array()[i][0] = !elems_P0.is_ghost(i);
    }
  }

  solver::actions::Proto::ProtoAction::execute();
  
  //TODO: Stop this from counting overlapping faces twice
  Real global_integral = m_integral_value;
  Real global_area = m_area;
  
  if(common::PE::Comm::instance().is_active())
  {
    common::PE::Comm::instance().all_reduce(common::PE::plus(), &m_integral_value, 1, &global_integral);
    common::PE::Comm::instance().all_reduce(common::PE::plus(), &m_area, 1, &global_area);
  }
  m_result = global_integral / global_area;

  m_changing_result = true;
  options().set("result", m_result);
  m_changing_result = false;
  
  if(is_not_null(m_history))
  {
    m_history->set("bulk_velocity", m_result);
    m_history->save_entry();
  }
}

void BulkVelocity::trigger_result()
{
  if(!m_changing_result)
    throw common::BadValue(FromHere(), "Option result was changed, but this is not allowed!");
}

} // namespace UFEM

} // namespace cf3
