// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/Builder.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
#include "common/FindComponents.hpp"

#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/PE/Buffer.hpp"

#include "math/MatrixTypesConversion.hpp"
#include "math/VariablesDescriptor.hpp"
#include "math/Consts.hpp"

#include "solver/actions/Probe.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/PointInterpolator.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;
using namespace common::XML;
using namespace mesh;

common::ComponentBuilder < Probe, common::Action, solver::actions::LibActions > Probe_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Probe::Probe( const std::string& name  ) : common::Action(name)
{
  mark_basic(); // by default probes are visible

  properties()["brief"] = std::string("Probe to interpolate field values to a given coordinate");
  std::string description =
      "Configure a coordinate and dictionary, and the probe will interpolate all found values";
  properties()["description"] = description;
  
  options().add("coordinate",std::vector<Real>())
    .pretty_name("Coordinate")
    .description("Coordinate to interpolate fields to")
    .mark_basic();
    
  options().add("dict",m_dict)
      .description("Dictionary that will be probed")
      .link_to(&m_dict)
      .attach_trigger( boost::bind( &Probe::configure_point_interpolator, this ) );

  regist_signal ( "create_post_processor" )
      .description( "Create a post processing action after probe execution" )
      .pretty_name("Create Post Processor" )
      .connect   ( boost::bind ( &Probe::signal_create_post_processor,    this, _1 ) )
      .signature ( boost::bind ( &Probe::signature_create_post_processor, this, _1 ) );


  m_point_interpolator = create_component<PointInterpolator>("point_interpolator");
  m_variables = create_component<math::VariablesDescriptor>("variables");
}

////////////////////////////////////////////////////////////////////////////////

void Probe::configure_point_interpolator()
{
  m_point_interpolator->options().set("dict",m_dict);
}

////////////////////////////////////////////////////////////////////////////////

void Probe::execute()
{
  if ( is_null(m_dict) )
    throw SetupError(FromHere(), "Option \"dict\" was not configured in "+uri().string());

  // Take the coordinate from the options
  std::vector<Real> opt_coord = options().value< std::vector<Real> >("coordinate");
  RealVector coord(opt_coord.size());
  math::copy(opt_coord,coord);

  // Find interpolation data for this coordinate
  SpaceElem m_element;
  std::vector<SpaceElem> m_stencil;
  std::vector<Uint> m_points;
  std::vector<Real> m_weights;

  int found = m_point_interpolator->compute_storage(coord,m_element,m_stencil,m_points,m_weights);

//  std::cout << PE::Comm::instance().rank() << ":  found = " << found << std::endl;

  int found_on_proc = found ? PE::Comm::instance().rank() : -1;

  if (PE::Comm::instance().is_active())
    PE::Comm::instance().all_reduce(PE::max(), &found_on_proc, 1, &found_on_proc);

//  std::cout << PE::Comm::instance().rank() << ":  found_on_proc = " << found_on_proc << std::endl;

  if (found_on_proc<0)
    throw SetupError(FromHere(),"Cannot probe: coordinate ("+to_str(opt_coord)+") lies outside the domain");

  PE::Buffer elem_comp_buffer;
  if (found)
  {
    elem_comp_buffer << m_element.comp->uri().path() << m_element.glb_idx();
  }
  elem_comp_buffer.broadcast(found_on_proc);
  std::string elem_comp;
  Uint glb_idx;
  elem_comp_buffer >> elem_comp >> glb_idx;

  properties()["space"]=elem_comp;
  properties()["glb_elem_idx"]=glb_idx;

//  std::cout << PE::Comm::instance().rank() << ":  elem_comp = " << elem_comp << std::endl;
//  std::cout << PE::Comm::instance().rank() << ":  glb_idx = " << glb_idx << std::endl;

  boost_foreach (const Handle<Field>& field, m_dict->fields())
  {

    // Interpolate each field to the given point
    std::vector<Real> interpolated(field->row_size());

    if (found)
    {
      for(Uint v=0; v<interpolated.size(); ++v)
      {
        interpolated[v]=0.;
        for(Uint i=0; i<m_points.size(); ++i)
        {
          interpolated[v] += field->array()[m_points[i]][v] * m_weights[i];
        }
      }
    }

    PE::Comm::instance().broadcast(interpolated,interpolated,found_on_proc);

    // Set interpolated variables as properties
    for (Uint var_idx=0; var_idx<field->nb_vars(); ++var_idx)
    {
      Uint var_begin  = field->descriptor().offset(var_idx);
      Uint var_length = field->descriptor().var_length(var_idx);
      if (var_length==1)
      {
        set(field->descriptor().user_variable_name(var_idx) , interpolated[var_begin]);
      }
      else
      {
        for (Uint i=0; i<var_length; ++i)
        {
          set(field->descriptor().user_variable_name(var_idx)+"["+to_str(i)+"]" , interpolated[var_begin+i]);
        }
      }
    }
  }

  // Do all post-processing actions, which could add more properties to the probe,
  // or other things, such as log a variable in a History component, ...
  boost_foreach (common::Action& action, find_components<common::Action>(*this))
  {
    action.execute();
  }

}

////////////////////////////////////////////////////////////////////////////////

void Probe::set(const std::string& var_name, const Real& var_value)
{
  if (properties().check(var_name) == false)
  {
    if (m_variables->nb_vars() == 0)
    {
      m_variables->options().set("dimension",(Uint)options().value<std::vector<Real> >("coordinate").size());
    }

    m_variables->push_back(var_name,math::VariablesDescriptor::Dimensionalities::SCALAR);
  }
  properties()[var_name] = var_value;
}

////////////////////////////////////////////////////////////////////////////////

void Probe::set(const std::string& var_name, const std::vector<Real>& var_values)
{
  for (Uint i=0; i<var_values.size(); ++i)
  {
    set(var_name+"["+to_str(i)+"]",var_values[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////

Handle<ProbePostProcessor> Probe::create_post_processor(const std::string& name, const std::string& builder)
{
  Handle<ProbePostProcessor> p = create_component(name,builder)->handle<ProbePostProcessor>();
  p->options().set("probe",this->handle());
  return p;
}

////////////////////////////////////////////////////////////////////////////////

void Probe::signal_create_post_processor(SignalArgs &args)
{
  SignalOptions signal_options(args);

  Handle<ProbePostProcessor> pp =
      create_post_processor(signal_options.value<std::string>("name"),
                            signal_options.value<std::string>("type"));

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", pp->uri());
}

////////////////////////////////////////////////////////////////////////////////

void Probe::signature_create_post_processor(SignalArgs &args)
{
  SignalOptions options(args);

  options.add("name", std::string("function"));
  options.add("type", std::string("cf3.mesh.ProbeFunction"));
}

////////////////////////////////////////////////////////////////////////////////

Probe::~Probe() {}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ProbePostProcessor::ProbePostProcessor(const std::string &name) : common::Action(name)
{
  options().add("probe",m_probe).link_to(&m_probe);
}

////////////////////////////////////////////////////////////////////////////////

void ProbePostProcessor::set(const std::string& var_name, const Real& var_value)
{
  m_probe->set(var_name,var_value);
}

////////////////////////////////////////////////////////////////////////////////

void ProbePostProcessor::set(const std::string& var_name, const std::vector<Real>& var_values)
{
  m_probe->set(var_name,var_values);
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
