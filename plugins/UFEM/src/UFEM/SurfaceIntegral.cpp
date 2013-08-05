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

#include "SurfaceIntegral.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < SurfaceIntegral, common::Action, LibUFEM > SurfaceIntegral_Builder;

SurfaceIntegral::SurfaceIntegral(const std::string& name) :
  ProtoAction(name),
  m_changing_result(false)
{    
  options().add("history",m_history)
      .pretty_name("History")
      .description("History component used to log the history of the integral value.")
      .link_to(&m_history)
      .mark_basic();
      
  regist_signal ( "set_field" )
    .connect( boost::bind ( &SurfaceIntegral::signal_set_field, this, _1 ) )
    .description( "Set up the model using a specific solver" )
    .pretty_name( "Setup" )
    .signature( boost::bind ( &SurfaceIntegral::signature_set_field, this, _1 ) );
}

SurfaceIntegral::~SurfaceIntegral()
{
}

void SurfaceIntegral::set_field(const std::string& variable_name, const std::string& tag)
{
  using boost::proto::lit;
  
  const Uint dim = physical_model().ndim();
  if(options().check("result"))
    options().erase("result");

  typedef boost::mpl::vector6<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Quad3D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP0::Line, mesh::LagrangeP0::Quad, mesh::LagrangeP0::Triag> element_types;
  
  // Indicate is an element is a owned by the current process
  FieldVariable<1, ScalarField> local("is_local_element", "surface_integrator", mesh::LagrangeP0::LibLagrangeP0::library_namespace());

  math::VariablesDescriptor& descriptor = common::find_component_with_tag<math::VariablesDescriptor>(physical_model().variable_manager(), tag);
  if(descriptor.dimensionality(variable_name) == math::VariablesDescriptor::Dimensionalities::SCALAR)
  {
    if(dim == 1)
      options().add("result", 0.).pretty_name("Result").description("Result of the last integral computation").mark_basic().attach_trigger(boost::bind(&SurfaceIntegral::trigger_result, this));
    else
      options().add("result", std::vector<Real>(dim)).pretty_name("Result").description("Result of the last integral computation").mark_basic().attach_trigger(boost::bind(&SurfaceIntegral::trigger_result, this));
    
    FieldVariable<0, ScalarField> s(variable_name, tag);
    
    m_integral_value.resize(dim);
    m_integral_value.setZero();
    
    set_expression(elements_expression(element_types(),
      lit(m_integral_value) += local*integral<1>(s * normal)));
  }
  else if(descriptor.dimensionality(variable_name) == math::VariablesDescriptor::Dimensionalities::VECTOR)
  {
    m_integral_value.resize(1);
    m_integral_value.setZero();
    options().add("result", 0.).pretty_name("Result").description("Result of the last integral computation").mark_basic().attach_trigger(boost::bind(&SurfaceIntegral::trigger_result, this));
    
    FieldVariable<0, VectorField> v(variable_name, tag);
    
    set_expression(elements_expression(element_types(),
      lit(m_integral_value) += local*integral<1>(v * normal)));
  }
  else
  {
    throw common::SetupError(FromHere(), "Variable type " + math::VariablesDescriptor::Dimensionalities::Convert::instance().to_str(descriptor.dimensionality(variable_name)) + " for variable " + variable_name + " is neither SCALAR nor VECTOR");
  }
}

void SurfaceIntegral::signal_set_field(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  set_field(options.value<std::string>("variable_name"), options.value<std::string>("field_tag"));
}

void SurfaceIntegral::signature_set_field(common::SignalArgs& args)
{
  common::XML::SignalOptions options(args);
  
  options.add("variable_name", "SomeScalar")
    .pretty_name("Variable Name")
    .description("Name of the variable to use");

  options.add("field_tag", "SomeFieldTag")
    .pretty_name("Field Tag")
    .description("Tag for the field to use");
}


void SurfaceIntegral::execute()
{
  if(m_loop_regions.empty())
  {
    CFwarn << "SurfaceIntegral has no regions" << CFendl;
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
  
  const Uint dim = m_integral_value.size();
  
  //TODO: Stop this from counting overlapping faces twice
  std::vector<Real> local_v(dim);
  for(Uint i = 0; i != dim; ++i)
    local_v[i] = m_integral_value[i];
  std::vector<Real> global_v(local_v.begin(), local_v.end());
  
  if(common::PE::Comm::instance().is_active())
  {
    common::PE::Comm::instance().all_reduce(common::PE::plus(), local_v, global_v);
  }

  m_changing_result = true;
  if(dim == 1)
    options().set("result", global_v[0]);
  else
    options().set("result", global_v);
  m_changing_result = false;
  
  if(is_not_null(m_history))
  {
    m_history->set(m_variable_name, global_v);
    m_history->save_entry();
  }
}

void SurfaceIntegral::trigger_result()
{
  if(!m_changing_result)
    throw common::BadValue(FromHere(), "Option result was changed, but this is not allowed!");
}

} // namespace UFEM

} // namespace cf3
