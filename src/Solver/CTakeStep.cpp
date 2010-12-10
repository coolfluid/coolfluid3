// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Math/MathConsts.hpp"

#include "Solver/CTakeStep.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Actions;

namespace CF {
namespace Solver {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CTakeStep, CLoopOperation, LibSolver > CTakeStep_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CTakeStep::CTakeStep ( const std::string& name ) : 
  CNodeOperation(name)
{
  // actions
  m_properties.add_option<OptionT<std::string> > ("SolutionField","Solution Field for calculation", "")->mark_basic();
  m_properties.add_option< OptionT<std::string> > ("ResidualField","Residual Field updated after calculation", "")->mark_basic();
  m_properties.add_option< OptionT<std::string> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", "")->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////////////

void CTakeStep::execute()
{  
  cf_assert( is_not_null(m_loop_helper) );
  m_loop_helper->solution[m_idx][0] += - ( 1./m_loop_helper->inverse_updatecoeff[m_idx][0] ) * m_loop_helper->residual[m_idx][0];
}

/////////////////////////////////////////////////////////////////////////////////////

void CTakeStep::create_loop_helper (CElements& geometry_elements )
{
  m_loop_helper.reset( new LoopHelper(geometry_elements , *this ) );
}

/////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CTakeStep::loop_list() const
{
  cf_assert( is_not_null(m_loop_helper) );
  return m_loop_helper->node_list;
}
	
////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

