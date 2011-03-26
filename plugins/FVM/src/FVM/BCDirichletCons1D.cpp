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

#include "FVM/BCDirichletCons1D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BCDirichletCons1D, BC, LibFVM > BCDirichletCons1D_Builder;
Common::ComponentBuilder < BCDirichletCons1D, CAction, LibFVM > BCDirichletCons1D_CAction_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BCDirichletCons1D::BCDirichletCons1D ( const std::string& name ) : 
  BC(name),
  m_connected_solution("solution_view")
{
  mark_basic();
  // options
  m_properties.add_option(OptionURI::create("solution","Solution","Cell based solution","cpath:/",URI::Scheme::CPATH))
    ->attach_trigger ( boost::bind ( &BCDirichletCons1D::config_solution,   this ) )
    ->add_tag("solution");

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &BCDirichletCons1D::trigger_elements,   this ) );

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

void BCDirichletCons1D::config_solution()
{
  URI uri;  property("solution").put_value(uri);
  CField::Ptr comp = Core::instance().root()->access_component_ptr(uri)->as_ptr<CField>();
  if ( is_null(comp) ) throw CastingFailed (FromHere(), "Field must be of a CField or derived type");
  m_connected_solution.set_field(comp);
}

////////////////////////////////////////////////////////////////////////////////

void BCDirichletCons1D::trigger_elements()
{
  m_can_start_loop = m_connected_solution.set_elements(elements());
}

/////////////////////////////////////////////////////////////////////////////////////

void BCDirichletCons1D::execute()
{
  std::vector<CTable<Real>::Row> solution = m_connected_solution[idx()];
  solution[FIRST][0] = m_rho;
  solution[FIRST][1] = m_rho*m_u;
  solution[FIRST][2] = m_p/m_gm1 + 0.5*m_rho*m_u*m_u;
  
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

