// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CRegion.hpp"

#include "Actions/CTakeStep.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CTakeStep, CAction, LibActions, NB_ARGS_1 > CTakeStepProvider( "CTakeStep" );

///////////////////////////////////////////////////////////////////////////////////////
  
void CTakeStep::defineConfigProperties( Common::PropertyList& options )
{
  options.add_option< OptionT<URI> > ("SolutionField","Solution Field for calculation", URI("cpath://"))->mark_basic();
  options.add_option< OptionT<URI> > ("ResidualField","Residual Field updated after calculation", URI("cpath://"))->mark_basic();
  options.add_option< OptionT<URI> > ("InverseUpdateCoeff","Inverse update coefficient Field updated after calculation", URI("cpath://"))->mark_basic();
}

///////////////////////////////////////////////////////////////////////////////////////

CTakeStep::CTakeStep ( const CName& name ) : 
  CAction(name)
{
  BUILD_COMPONENT;
  m_property_list["SolutionField"].as_option().attach_trigger ( boost::bind ( &CTakeStep::trigger_SolutionField,   this ) );  
  m_property_list["ResidualField"].as_option().attach_trigger ( boost::bind ( &CTakeStep::trigger_ResidualField,   this ) );  
  m_property_list["InverseUpdateCoeff"].as_option().attach_trigger ( boost::bind ( &CTakeStep::trigger_InverseUpdateCoeff,   this ) );  
}

/////////////////////////////////////////////////////////////////////////////////////

void CTakeStep::trigger_SolutionField()
{
  CPath field_path (property("SolutionField").value<URI>());
  m_solution_field = look_component_type<CField>(field_path);
}

void CTakeStep::trigger_ResidualField()
{
  CPath field_path (property("ResidualField").value<URI>());
  m_residual_field = look_component_type<CField>(field_path);
}

void CTakeStep::trigger_InverseUpdateCoeff()
{
  CPath field_path (property("InverseUpdateCoeff").value<URI>());
  m_inverseUpdateCoeff = look_component_type<CField>(field_path);
}

/////////////////////////////////////////////////////////////////////////////////////

void CTakeStep::go_deeper(CField& driving_field, std::vector<CField::Ptr>& driven_fields)
{  
  BOOST_FOREACH(CField::Ptr& driven_field, driven_fields)
  {
    driven_field = driving_field.support().get_field(driven_field->field_name()).get_type<CField>();
  }
  
  if(filtered_range_typed<CArray>(driving_field,IsComponentTag("field_data")).empty())
  {
    BOOST_FOREACH(CField& subfield, range_typed<CField>(driving_field))
    {      
      go_deeper(subfield,driven_fields);
    }
  }
  else
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(driving_field,*driven_fields[0],*driven_fields[1]) );
    
    for(m_point_idx=0; m_point_idx<data->solution.size(); ++m_point_idx)
    {
      execute_impl();
    }
  }
}

void CTakeStep::execute_impl()
{
  data->solution[m_point_idx][0] += - ( 1./data->inverse_updatecoeff[m_point_idx][0] ) * data->residual[m_point_idx][0]; 
}

void CTakeStep::execute()
{  
  std::vector<CField::Ptr> driven_fields(2);
  driven_fields[0] = m_residual_field;
  driven_fields[1] = m_inverseUpdateCoeff;
  go_deeper(*m_solution_field,driven_fields);
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////////

