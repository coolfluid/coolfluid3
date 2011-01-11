// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Solver/Actions/CSetFieldValues.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSetFieldValues, CLoopOperation, LibActions > CSetFieldValues_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
CSetFieldValues::CSetFieldValues ( const std::string& name ) : 
  CNodeOperation(name)
{
  // options
  m_properties.add_option< OptionT<std::string> > ("Field","Field to output", "")->mark_basic();
}
	
/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::execute()
{
	const CF::Real x = m_loop_helper->coordinates[m_idx][XX];
	//const CF::Real y = coordinates[n][YY];
	
  const Uint row_size = m_loop_helper->field_data.row_size();
	for (Uint i = 0; i != row_size; ++i)
	{
		if (x >= -1.4 && x <= -0.6)
			m_loop_helper->field_data[m_idx][i] = 0.5*(cos(3.141592*(x+1.0)/0.4)+1.0);
		else
			m_loop_helper->field_data[m_idx][i] = 0.0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::create_loop_helper (CElements& geometry_elements )
{
	m_loop_helper = boost::shared_ptr<LoopHelper> ( new LoopHelper(geometry_elements , *this ) );
}

/////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CSetFieldValues::loop_list() const
{
	return m_loop_helper->used_nodes;
}
	
////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

