// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Log.hpp"

#include "Solver/ForwardEuler.hpp"
#include "Solver/CDiscretization.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"

#include "Actions/CLoop.hpp"
#include "Actions/CForAllNodes.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;
using namespace Actions;

Common::ComponentBuilder < ForwardEuler, CIterativeSolver, LibSolver > ForwardEuler_Builder;

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::ForwardEuler ( const std::string& name  ) :
  CIterativeSolver ( name )
{
  properties()["brief"] = std::string("Iterative Solver component");
  properties()["description"] = std::string("Forward Euler Time Stepper");
  
  std::vector< URI > dummy;
  m_properties.add_option< OptionArrayT < URI > > ("Regions", "Regions to solve for", dummy)->mark_basic();
  
  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &ForwardEuler::solve, this ) );
  // signal("solve").signature
  //     .insert<URI>("Domain", "Domain to load mesh into" )
  //     .insert_array<URI>( "Files" , "Files to read" );
  
}

////////////////////////////////////////////////////////////////////////////////

ForwardEuler::~ForwardEuler()
{
}

////////////////////////////////////////////////////////////////////////////////

CDiscretization& ForwardEuler::discretization_method()
{
  return find_component<CDiscretization>(*this);
}

//////////////////////////////////////////////////////////////////////////////

void ForwardEuler::solve()
{
    CMesh& mesh = find_component<CMesh>(*Core::instance().root());
    
    CField::Ptr solution = find_component_ptr_with_name<CField>(mesh,"solution");
    if ( is_null(solution) )
      solution = mesh.create_field("solution",1,CField::NODE_BASED).as_type<CField>();
    m_solution_field->link_to(solution);

    CField::Ptr residual = find_component_ptr_with_name<CField>(mesh,"residual");
    if ( is_null(residual) )
      residual = mesh.create_field("residual",1,CField::NODE_BASED).as_type<CField>();
    m_residual_field->link_to(residual);

    CField::Ptr inverse_updatecoeff = find_component_ptr_with_name<CField>(mesh,"inverse_updatecoeff");
    if ( is_null(inverse_updatecoeff) )
      inverse_updatecoeff = mesh.create_field("inverse_updatecoeff",1,CField::NODE_BASED).as_type<CField>();
    m_update_coeff_field->link_to(inverse_updatecoeff);

    CLoop::Ptr take_step = create_component_type<CForAllNodes>("take_step");
    take_step->create_action("CF.Actions.CTakeStep");
  	take_step->configure_property("Regions",property("Regions").value<URI>());
    take_step->action("CF.Actions.CTakeStep").configure_property("SolutionField",std::string("solution"));
    take_step->action("CF.Actions.CTakeStep").configure_property("ResidualField",std::string("residual"));
    take_step->action("CF.Actions.CTakeStep").configure_property("InverseUpdateCoeff",std::string("inverse_updatecoeff"));

    for ( Uint iter = 0; iter < m_nb_iter;  ++iter)
    {
      // update coefficient and residual to zero
      // Set the field data of the source field
      BOOST_FOREACH(CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_solution_field->get(), "node_data"))
        for (Uint i=0; i<node_data.size(); ++i)
    			node_data[i][0]=0;
      BOOST_FOREACH(CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_update_coeff_field->get(),"node_data"))
        for (Uint i=0; i<node_data.size(); ++i)
    			node_data[i][0]=0;

      // compute RHS
      discretization_method().compute_RHS();  

      // explicit update
      take_step->execute();

      // compute norm
      Real rhs_L2=0;
      Uint dof=0;
      BOOST_FOREACH(CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_residual_field.get(),"node_data"))
      {    
        for (Uint i=0; i<node_data.size(); ++i)
    		{
          rhs_L2 += node_data[i][0]*node_data[i][0];
          dof++;
    		}
      }
      rhs_L2 = sqrt(rhs_L2)/dof;

      // output convergence info
      CFinfo << "Iter [" << std::setw(4) << iter << "] L2(rhs) [" << std::setw(12) << rhs_L2 << "]" << CFendl;
    }
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
