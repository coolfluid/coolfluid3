// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"

#include "Math/MathConsts.hpp"

#include "Actions/CTakeStep.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CTakeStep, CLoopOperation, LibActions > CTakeStep_Builder( "CTakeStep" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CTakeStep::define_config_properties( Common::PropertyList& options )
{
  options.add_option< OptionT<std::string> > ("SolutionField","Solution Field for calculation", "")->mark_basic();
  options.add_option< OptionT<std::string> > ("ResidualField","Residual Field updated after calculation", "")->mark_basic();
  options.add_option< OptionT<std::string> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", "")->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

CTakeStep::CTakeStep ( const std::string& name ) : 
  CLoopOperation(name)
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////////////

void CTakeStep::execute()
{  
  data->solution[m_idx][0] += - ( 1./data->inverse_updatecoeff[m_idx][0] ) * data->residual[m_idx][0]; 

}

/////////////////////////////////////////////////////////////////////////////////////

void CTakeStep::set_loophelper (CElements& geometry_elements )
{
	data = boost::shared_ptr<LoopHelper> ( new LoopHelper(geometry_elements , *this ) );
}

/////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CTakeStep::loop_list()
{
	return data->node_list;
}
	
////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

