// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "FVM/ComputeFlux.hpp"

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

  m_solution = create_static_component<CFieldView>("solution_view");
  m_residual = create_static_component<CFieldView>("residual_view");
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_solution->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::config_residual()
{
  URI uri;  property("Residual").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_residual->set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::trigger_elements()
{
  m_solution->set_elements(elements());
  m_residual->set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void ComputeFlux::execute()
{
  // idx() is the index that is set using the function set_loop_idx() or configuration LoopIndex
  // 2) as simple field --> extra index for multiple variables per field
  CFieldView& residual = *m_residual;
  residual[idx()][0] = 1.;
  residual[idx()][1] = 1.;
  residual[idx()][2] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

