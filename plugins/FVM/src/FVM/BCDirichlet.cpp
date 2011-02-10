// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "FVM/BCDirichlet.hpp"
#include "FVM/RoeFluxSplitter.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BCDirichlet, CAction, LibFVM > BCDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BCDirichlet::BCDirichlet ( const std::string& name ) : 
  CLoopOperation(name)
{
  mark_basic();
  // options
  m_properties.add_option< OptionURI > ("Solution","Cell based solution", URI("cpath:"));
  m_properties["Solution" ].as_option().attach_trigger ( boost::bind ( &BCDirichlet::config_solution,   this ) );
  
  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &BCDirichlet::trigger_elements,   this ) );

  m_rho=1.225;
  m_u=0.;
  m_p=101300;
  m_gm1 = 0.4;
  
  m_properties.add_option< OptionT<Real> >("rho","density",m_rho)->mark_basic();
  m_properties.add_option< OptionT<Real> >("u","velocity",m_u)->mark_basic();
  m_properties.add_option< OptionT<Real> >("p","pressure",m_p)->mark_basic();
  
  m_properties["rho"].as_option().link_to(&m_rho);
  m_properties["u"].as_option().link_to(&m_u);
  m_properties["p"].as_option().link_to(&m_p);
  
  
}

////////////////////////////////////////////////////////////////////////////////

void BCDirichlet::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField2::Ptr comp = Core::instance().root()->look_component<CField2>(uri);
  if ( is_null(comp) )
    throw CastingFailed (FromHere(), "Field must be of a CField2 or derived type");
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCDirichlet::trigger_elements()
{
  m_connected_solution.set_elements(elements().as_type<CFaces>());
}

/////////////////////////////////////////////////////////////////////////////////////

void BCDirichlet::execute()
{
  m_connected_solution[idx()][0] = m_rho;
  m_connected_solution[idx()][1] = m_rho*m_u;
  m_connected_solution[idx()][2] = m_p/m_gm1 + 0.5*m_rho*m_u*m_u;
  
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

