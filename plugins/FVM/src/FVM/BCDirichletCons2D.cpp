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
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"

#include "FVM/BCDirichletCons2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BCDirichletCons2D, CAction, LibFVM > BCDirichletCons2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BCDirichletCons2D::BCDirichletCons2D ( const std::string& name ) : 
  CLoopOperation(name),
  m_connected_solution("solution_view")
{
  mark_basic();
  // options
  m_properties.add_option(OptionURI::create("Solution","Cell based solution","cpath:/",URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &BCDirichletCons2D::config_solution,   this ) )
    ->add_tag("solution");

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &BCDirichletCons2D::trigger_elements,   this ) );

  m_rho=1.225;
  m_u=0.;
  m_v=0.;
  m_p=101300;
  m_gm1 = 0.4;
  
  m_properties.add_option< OptionT<Real> >("rho","density",m_rho)->mark_basic();
  m_properties.add_option< OptionT<Real> >("u","x-velocity",m_u)->mark_basic();
  m_properties.add_option< OptionT<Real> >("v","y-velocity",m_v)->mark_basic();
  m_properties.add_option< OptionT<Real> >("p","pressure",m_p)->mark_basic();
  
  m_properties["rho"].as_option().link_to(&m_rho);
  m_properties["u"].as_option().link_to(&m_u);
  m_properties["v"].as_option().link_to(&m_u);
  m_properties["p"].as_option().link_to(&m_p);
  
  
}

////////////////////////////////////////////////////////////////////////////////

void BCDirichletCons2D::config_solution()
{
  URI uri;  property("Solution").put_value(uri);
  CField::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) ) throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCDirichletCons2D::trigger_elements()
{
  m_can_start_loop = m_connected_solution.set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void BCDirichletCons2D::execute()
{
  m_connected_solution[idx()][INNER][0] = m_rho;
  m_connected_solution[idx()][INNER][1] = m_rho*m_u;
  m_connected_solution[idx()][INNER][2] = m_rho*m_v;
  m_connected_solution[idx()][INNER][3] = m_p/m_gm1 + 0.5*m_rho*(m_u*m_u+m_v*m_v);
  
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

