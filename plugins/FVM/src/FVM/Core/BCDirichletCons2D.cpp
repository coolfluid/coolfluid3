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
#include "Mesh/Field.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"

#include "FVM/Core/BCDirichletCons2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BCDirichletCons2D, BC, LibCore > BCDirichletCons2D_Builder;
Common::ComponentBuilder < BCDirichletCons2D, CAction, LibCore > BCDirichletCons2D_CAction_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BCDirichletCons2D::BCDirichletCons2D ( const std::string& name ) :
  BC(name),
  m_connected_solution("solution_view")
{
  mark_basic();
  // options
  m_options.add_option(OptionURI::create("solution", "cpath:/", URI::Scheme::CPATH))
      ->description("Cell based solution")
      ->pretty_name("Solution")
      ->attach_trigger ( boost::bind ( &BCDirichletCons2D::config_solution,   this ) );

  m_options["Elements"].attach_trigger ( boost::bind ( &BCDirichletCons2D::trigger_elements, this ) );

  m_rho=1.225;
  m_u=0.;
  m_v=0.;
  m_p=101300;
  m_gm1 = 0.4;

  m_options.add_option< OptionT<Real> >("rho", m_rho)->description("density")->mark_basic();
  m_options.add_option< OptionT<Real> >("u", m_u)->description("x-velocity")->mark_basic();
  m_options.add_option< OptionT<Real> >("v", m_v)->description("y-velocity")->mark_basic();
  m_options.add_option< OptionT<Real> >("p", m_p)->description("pressure")->mark_basic();

  m_options["rho"].link_to(&m_rho);
  m_options["u"].link_to(&m_u);
  m_options["v"].link_to(&m_u);
  m_options["p"].link_to(&m_p);


}

////////////////////////////////////////////////////////////////////////////////

void BCDirichletCons2D::config_solution()
{
  URI uri;  option("solution").put_value(uri);
  Field::Ptr comp = Common::Core::instance().root().access_component_ptr(uri)->as_ptr<Field>();
  if ( is_null(comp) ) throw CastingFailed (FromHere(), "Field must be of a Field or derived type");
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

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

