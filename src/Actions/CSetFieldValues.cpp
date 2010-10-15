// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Actions/CSetFieldValues.hpp"
#include "Mesh/CFieldElements.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CSetFieldValues, CLoopOperation, LibActions, NB_ARGS_1 > CSetFieldValuesProvider( "CSetFieldValues" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CSetFieldValues::defineConfigProperties( Common::PropertyList& options )
{
  options.add_option< OptionT<std::string> > ("Field","Field to output", "")->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

CSetFieldValues::CSetFieldValues ( const CName& name ) : 
  CLoopOperation(name)
{
  BUILD_COMPONENT;
}
	
/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::execute()
{
	const CF::Real x = data->coordinates[m_idx][XX];
	//const CF::Real y = coordinates[n][YY];
	
	const Uint row_size = data->field_data.row_size();
	for (Uint i = 0; i != row_size; ++i)
	{
		if (x >= -1.4 && x <= -0.6)
			data->field_data[m_idx][i] = 0.5*(cos(3.141592*(x+1.0)/0.4)+1.0);
		else
			data->field_data[m_idx][i] = 0.0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void CSetFieldValues::set_loophelper (CElements& geometry_elements )
{
	data = boost::shared_ptr<LoopHelper> ( new LoopHelper(geometry_elements , *this ) );
}

/////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CSetFieldValues::loop_list()
{
	return data->node_list;
}
	
////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

