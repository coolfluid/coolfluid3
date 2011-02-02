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
  CLoopOperation(name)
{
  // options
  m_properties.add_option< OptionURI > ("Solution","Cell based solution", URI("cpath:"))->mark_basic();
  m_properties["Solution" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_solution,   this ) );

  m_properties.add_option< OptionURI > ("Residual","Residual to compute", URI("cpath:"))->mark_basic();
  m_properties["Residual" ].as_option().attach_trigger ( boost::bind ( &ComputeFlux::config_residual,   this ) );
  
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &ComputeFlux::trigger_elements,   this ) );

  m_fluxsplitter = create_static_component<RoeFluxSplitter>("Roe_fluxsplitter");
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_residual()
{
  URI uri;  property("Residual").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_connected_residual.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::trigger_elements()
{
  m_connected_solution.set_elements(elements().as_type<CCellFaces>());
  m_connected_residual.set_elements(elements().as_type<CCellFaces>());
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex
  // 2) as simple field --> extra index for multiple variables per field
  // CFieldView& residual = *m_residual;
  // residual[idx()][0] = 1.;
  // residual[idx()][1] = 1.;
  // residual[idx()][2] = 1.;
  enum {LEFT=0,RIGHT=1};

  RealVector U_L(3), U_R(3);
  for (Uint i=0; i<3; ++i)
  {
    U_L[i]=m_connected_solution[idx()][LEFT ][i];
    U_R[i]=m_connected_solution[idx()][RIGHT][i];
  }
  
  RealVector flux = m_fluxsplitter->solve(U_L,U_R);
  
  for (Uint i=0; i<3; ++i)
  {
    m_connected_residual[idx()][LEFT ][i] -= flux[i]; // flux going OUT of left cell
    m_connected_residual[idx()][RIGHT][i] += flux[i]; // flux going IN to right cell
  }
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

