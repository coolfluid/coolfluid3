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

#include "FVM/ComputeFlux.hpp"
#include "FVM/RoeFluxSplitter.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ComputeFlux, CLoopOperation, LibFVM > ComputeFlux_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
ComputeFlux::ComputeFlux ( const std::string& name ) : 
  CLoopOperation(name),
  m_flux(3),
  m_normal(1),
  m_state_L(3),
  m_state_R(3)
{
  // options
  m_properties.add_option< OptionURI > ("Solution","Cell based solution", URI("cpath:"))->mark_basic();
  m_properties["Solution" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_solution,   this ) );

  m_properties.add_option< OptionURI > ("Residual","Residual to compute", URI("cpath:"))->mark_basic();
  m_properties["Residual" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_residual,   this ) );

  m_properties.add_option< OptionURI > ("Advection","Advection to compute", URI("cpath:"))->mark_basic();
  m_properties["Advection" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_advection,   this ) );

  m_properties.add_option< OptionURI > ("Area","Face area", URI("cpath:"))->mark_basic();
  m_properties["Area" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_area,   this ) );

  m_properties.add_option< OptionURI > ("FaceNormal","Unit normal to the face, outward from left cell", URI("cpath:"))->mark_basic();
  m_properties["FaceNormal" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_normal,   this ) );
  
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &ComputeFlux::trigger_elements,   this ) );

  m_fluxsplitter = create_static_component<RoeFluxSplitter>("Roe_fluxsplitter");
  m_face_area = create_static_component<CScalarFieldView>("face_area_view");
  m_face_normal = create_static_component<CFieldView>("face_normal_view");
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_residual()
{
  URI uri;  property("Residual").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  m_connected_residual.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_advection()
{
  URI uri;  property("Advection").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  m_connected_advection.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_area()
{
  URI uri;  property("Area").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  m_face_area->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_normal()
{
  URI uri;  property("FaceNormal").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  m_face_normal->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::trigger_elements()
{
  m_connected_solution.set_elements(elements().as_type<CCellFaces>());
  m_connected_residual.set_elements(elements().as_type<CCellFaces>());
  m_connected_advection.set_elements(elements().as_type<CCellFaces>());
  m_face_normal->set_elements(elements());
  m_face_area->set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::execute()
{
  // Copy the left and right states to a RealVector
  for (Uint i=0; i<3; ++i)
  {
    m_state_L[i]=m_connected_solution[idx()][LEFT ][i];
    m_state_R[i]=m_connected_solution[idx()][RIGHT][i];
  }
  
  // Copy the face normal to a RealVector
  m_normal[XX] = (*m_face_normal)[idx()][XX];


  // Solve the riemann problem on this face.
  m_fluxsplitter->solve( 
                         // intput
                         m_state_L,m_state_R,m_normal,
                         // output
                         m_flux, m_wave_speed_left, m_wave_speed_right 
                        );


  // accumulate fluxes to the residual
  for (Uint i=0; i<3; ++i)
  {
    m_connected_residual[idx()][LEFT ][i] -= m_flux[i]; // flux going OUT of left cell
    m_connected_residual[idx()][RIGHT][i] += m_flux[i]; // flux going IN to right cell
  }

  // accumulate most negative wave_speeds * area, for use in CFL condition
  m_connected_advection[idx()][LEFT ][0] -= std::min(m_wave_speed_left ,0.) * (*m_face_area)[idx()];
  m_connected_advection[idx()][RIGHT][0] -= std::min(m_wave_speed_right,0.) * (*m_face_area)[idx()];

}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

