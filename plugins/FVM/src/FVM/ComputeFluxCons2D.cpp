// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "Math/MathChecks.hpp"

#include "FVM/ComputeFluxCons2D.hpp"
#include "FVM/RoeFluxSplitterCons2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeFluxCons2D, CLoopOperation, LibFVM > ComputeFluxCons2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
ComputeFluxCons2D::ComputeFluxCons2D ( const std::string& name ) : 
  CLoopOperation(name),
  m_connected_residual("residual_view"),
  m_connected_solution("solution_view"),
  m_connected_wave_speed("wave_speed_view"),
  m_face_area("face_area_view"),  
  m_face_normal("face_normal_view"),
  m_flux(4),
  m_wave_speed_left(0),
  m_wave_speed_right(0),
  m_normal(2),
  m_state_L(4),
  m_state_R(4)
{
  // options
  m_properties.add_option(OptionURI::create("Solution","Cell based solution", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFluxCons2D::config_solution,   this ) )
    ->add_tag("solution");

  m_properties.add_option(OptionURI::create("Residual","Residual to compute", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFluxCons2D::config_residual,   this ) )
    ->add_tag("residual");

  m_properties.add_option(OptionURI::create("WaveSpeed","WaveSpeed to compute", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFluxCons2D::config_wave_speed,   this ) )
    ->add_tag("wave_speed");

  m_properties.add_option(OptionURI::create("Area","Face area", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFluxCons2D::config_area,   this ) )
    ->add_tag("area");

  m_properties.add_option(OptionURI::create("FaceNormal","Unit normal to the face, outward from left cell", URI("cpath:"), URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &ComputeFluxCons2D::config_normal,   this ) )
    ->add_tag("face_normal");
  
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &ComputeFluxCons2D::trigger_elements,   this ) );

  m_fluxsplitter = create_static_component<RoeFluxSplitterCons2D>("Roe_fluxsplitter");
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField2>();
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::config_residual()
{
  URI uri;  property("Residual").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField2>();
  m_connected_residual.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::config_wave_speed()
{
  URI uri;  property("WaveSpeed").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField2>();
  m_connected_wave_speed.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::config_area()
{
  URI uri;  property("Area").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField2>();
  m_face_area.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::config_normal()
{
  URI uri;  property("FaceNormal").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField2>();
  m_face_normal.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::trigger_elements()
{
    m_connected_solution.set_elements(elements());
    m_connected_residual.set_elements(elements());
    m_connected_wave_speed.set_elements(elements());
    m_face_normal.set_elements(elements());
    m_face_area.set_elements(elements());
    m_can_start_loop = true;
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeFluxCons2D::execute()
{
  // Copy the left and right states to a RealVector
  for (Uint i=0; i<m_flux.size(); ++i)
  {
    m_state_L[i]=m_connected_solution[idx()][LEFT ][i];
    m_state_R[i]=m_connected_solution[idx()][RIGHT][i];
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
    m_connected_residual[idx()][LEFT ][i] -= m_flux[i] * m_face_area[idx()]; // flux going OUT of left cell
    m_connected_residual[idx()][RIGHT][i] += m_flux[i] * m_face_area[idx()]; // flux going IN to right cell
  }

  // accumulate wave_speeds * area, for use in CFL condition
  m_connected_wave_speed[idx()][LEFT ][0] += std::max(m_wave_speed_left ,0.) * m_face_area[idx()];
  m_connected_wave_speed[idx()][RIGHT][0] += std::max(m_wave_speed_right,0.) * m_face_area[idx()]; 

}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

