// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "Math/MathChecks.hpp"
#include "Math/MathConsts.hpp"

#include "FVM/ComputeFlux.hpp"
#include "FVM/RiemannSolver.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;
using namespace CF::Math::MathConsts;
namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeFlux, CLoopOperation, LibFVM > ComputeFlux_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
ComputeFlux::ComputeFlux ( const std::string& name ) : 
  CLoopOperation(name),
  m_connected_residual("residual_view"),
  m_connected_solution("solution_view"),
  m_connected_wave_speed("wave_speed_view"),
  m_face_area("face_area_view"),  
  m_face_normal("face_normal_view"),
  m_wave_speed_left(0),
  m_wave_speed_right(0)
{
  // options
  m_properties.add_option(OptionURI::create("solution","Solution","Cell based solution", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFlux::config_solution,   this ) )
    ->add_tag("solution");

  m_properties.add_option(OptionURI::create("residual","Residual","Residual to compute", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFlux::config_residual,   this ) )
    ->add_tag("residual");

  m_properties.add_option(OptionURI::create("wave_speed","Wave Speed","Wave Speed to compute", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFlux::config_wave_speed,   this ) )
    ->add_tag("wave_speed");

  m_properties.add_option(OptionURI::create("area","Area","Face area", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFlux::config_area,   this ) )
    ->add_tag(Mesh::Tags::area());

  m_properties.add_option(OptionURI::create("face_normal","FaceNormal","Unit normal to the face, outward from left cell", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFlux::config_normal,   this ) )
    ->add_tag(Mesh::Tags::normal());
  
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &ComputeFlux::trigger_elements,   this ) );

}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_solution()
{
  URI uri;  property("solution").put_value(uri);
  CField::Ptr solution = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  m_connected_solution.set_field(solution);
  m_flux.resize(solution->data().row_size());
  m_normal.resize(m_flux.size()-2);
  m_state_L.resize(m_flux.size());
  m_state_R.resize(m_flux.size());
  
  if (is_null(m_fluxsplitter))
  {
    m_fluxsplitter = create_component_abstract_type<RiemannSolver>("CF.FVM.RoeCons"+to_str(Uint(m_normal.size()))+"D","Roe_fluxsplitter");
  }
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_residual()
{
  URI uri;  property("residual").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  m_connected_residual.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_wave_speed()
{
  URI uri;  property("wave_speed").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  m_connected_wave_speed.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_area()
{
  URI uri;  property("area").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  m_face_area.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_normal()
{
  URI uri;  property("face_normal").put_value(uri);
  CField::Ptr comp = Core::instance().root().access_component_ptr(uri)->as_ptr<CField>();
  m_face_normal.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::trigger_elements()
{
    m_connected_solution.set_elements(elements());
    m_connected_residual.set_elements(elements());
    m_connected_wave_speed.set_elements(elements());
    m_face_normal.set_elements(elements());
    m_face_area.set_elements(elements());
    m_can_start_loop = true;
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::execute()
{
  //CFinfo << "connected = " << to_vector(m_connected_solution.connected_idx(idx())).transpose() << CFendl;
  std::vector<CTable<Real>::Row> residual   = m_connected_residual[idx()];
  std::vector<CTable<Real>::Row> wave_speed = m_connected_wave_speed[idx()];
  std::vector<CTable<Real>::Row> solution   = m_connected_solution[idx()];
  const Real area = m_face_area[idx()];
  
  // Copy the left and right states to a RealVector
  for (Uint i=0; i<m_flux.size(); ++i)
  {
    m_state_L[i]=solution[LEFT ][i];
    m_state_R[i]=solution[RIGHT][i];
    
    /// @todo investigate why Eigen chokes on values such as 1e-107 for a state
    /// Eigen then raises a SIGFPE signal.
    if (std::abs(m_state_L[i]) < eps() ) m_state_L[i] = 0.;
    if (std::abs(m_state_R[i]) < eps() ) m_state_R[i] = 0.;
  }
  
  
  // Copy the face normal to a RealVector
  m_normal = to_vector(m_face_normal[idx()]);

  // Solve the riemann problem on this face.
  m_fluxsplitter->solve( 
                         // intput
                         m_state_L,m_state_R,m_normal,
                         // output
                         m_flux, m_wave_speed_left, m_wave_speed_right 
                        );


  // accumulate fluxes to the residual
  for (Uint i=0; i<m_flux.size(); ++i)
  {
    residual[LEFT][i]  -= m_flux[i] * area; // flux going OUT of left cell
    residual[RIGHT][i] += m_flux[i] * area; // flux going IN to right cell
  }

  // accumulate wave_speeds * area, for use in CFL condition
  wave_speed[LEFT ][0] += std::max(m_wave_speed_left ,0.) * area;
  wave_speed[RIGHT][0] += std::max(m_wave_speed_right,0.) * area; 

}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

