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
#include "Common/Foreach.hpp"
#include "Common/String/Conversion.hpp"

#include "Math/MathChecks.hpp"

#include "RDM/RungeKutta.hpp"
#include "Solver/CDiscretization.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

namespace CF {
namespace RDM {

using namespace Common;
using namespace Common::String;
using namespace Mesh;
using namespace Solver;
using namespace Solver::Actions;
using namespace Math::MathChecks;

Common::ComponentBuilder < RungeKutta, CIterativeSolver, LibRDM > RungeKutta_Builder;

////////////////////////////////////////////////////////////////////////////////

RungeKutta::RungeKutta ( const std::string& name  ) :
  CIterativeSolver ( name )
{
  properties()["brief"] = std::string("Iterative RDM component");
  properties()["description"] = std::string("Forward Euler Time Stepper");

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &RungeKutta::trigger_Domain,   this ) );


  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &RungeKutta::solve, this ) );
  // signal("solve").signature
  //     .insert<URI>("Domain", "Domain to load mesh into" )
  //     .insert_array<URI>( "Files" , "Files to read" );

  m_solution_field =     create_static_component<CLink>( "solution" );
  m_residual_field =     create_static_component<CLink>( "residual" );
  m_update_coeff_field = create_static_component<CLink>( "update_coeff" );

}

////////////////////////////////////////////////////////////////////////////////

RungeKutta::~RungeKutta()
{
}

////////////////////////////////////////////////////////////////////////////////

void RungeKutta::trigger_Domain()
{
  CDomain::Ptr domain = look_component<CDomain>( property("Domain").value<URI>() );
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not point to Domain");

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>( *domain );
  if (is_not_null(mesh))
  {

//    std::vector<URI> volume_regions;
//    boost_foreach( const CRegion& region, find_components_recursively_with_name<CRegion>(*mesh,"topology"))
//      volume_regions.push_back( region.full_path() );

    CFinfo << " - setting solution field" << CFendl;

    CField2::Ptr solution = find_component_ptr_with_name<CField2>(*mesh,"solution");
    if ( is_null(solution) )
      solution = mesh->create_field2("solution","PointBased","u[1]").as_type<CField2>();
    m_solution_field->link_to(solution);

    CFinfo << " - setting residual field" << CFendl;

    CField2::Ptr residual = find_component_ptr_with_name<CField2>(*mesh,"residual");
    if ( is_null(residual) )
      residual = mesh->create_field2("residual","PointBased","ru[1]").as_type<CField2>();
    m_residual_field->link_to(residual);

    CFinfo << " - setting update_coeff field" << CFendl;

    CField2::Ptr update_coeff = find_component_ptr_with_name<CField2>(*mesh,"update_coeff");
    if ( is_null(update_coeff) )
      update_coeff = mesh->create_field2("update_coeff","PointBased").as_type<CField2>();
    m_update_coeff_field->link_to(update_coeff);


    // configure DM mesh after fields are created since DM will look for them
    // on the attached configuration trigger
    discretization_method().configure_property( "Mesh" , mesh->full_path() );
  }
  else
  {
    CFinfo << "domain has no mesh " << CFendl;
    return;
  }
}

//////////////////////////////////////////////////////////////////////////////

CDiscretization& RungeKutta::discretization_method()
{
  return find_component<CDiscretization>(*this);
}

//////////////////////////////////////////////////////////////////////////////

void RungeKutta::solve()
{
  // ensure domain is sane
  CDomain::Ptr domain = look_component<CDomain>( property("Domain").value<URI>() );
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not poitn to Domain");

  CField2::Ptr solution     = m_solution_field->follow()->as_type<CField2>();
  CField2::Ptr residual     = m_residual_field->follow()->as_type<CField2>();
  CField2::Ptr update_coeff = m_update_coeff_field->follow()->as_type<CField2>();

  CFinfo << " - initializing solution" << CFendl;

  // initialize to zero condition
  /// @todo should be moved out of here
  boost_foreach (CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_solution_field->follow(), "node_data"))
  {
    for (Uint i=0; i<node_data.size(); ++i)
      node_data[i][0]=0;
  }

  CFinfo << " - starting iterative loop" << CFendl;

  for ( Uint iter = 1; iter <= m_nb_iter;  ++iter)
  {
    /// @todo move this into an action

    CFinfo << " --  cleaning residual field" << CFendl;

    // set update coefficient and residual to zero
    // Set the field data of the source field
    boost_foreach (CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_residual_field->follow(), "node_data"))
    {
      Uint size = node_data.size();
      for (Uint i=0; i<size; ++i)
        node_data[i][0]=0;
    }

    CFinfo << " --  cleaning update coeff" << CFendl;
    boost_foreach (CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_update_coeff_field->follow(),"node_data"))
    {
      Uint size = node_data.size();
      for (Uint i=0; i<size; ++i)
        node_data[i][0]=0;
    }

    // compute RHS
    CFinfo << " --  computing the rhs" << CFendl;
    discretization_method().compute_rhs();

    // CFL
    const Real CFL = 0.9;

    // explicit update
    CFinfo << " --  updating solution" << CFendl;
    const Uint nbdofs = solution->size();
    for (Uint i=0; i< nbdofs; ++i)
      (*solution)[i][0] += - ( CFL / (*update_coeff)[i][0] ) * (*residual)[i][0];

    CFinfo << " --  computing the norm" << CFendl;
    Real rhs_L2=0;
    Uint dof=0;
    boost_foreach (CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_residual_field->follow(),"node_data"))
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
    if ( is_nan(rhs_L2) || is_inf(rhs_L2) )
      throw FailedToConverge(FromHere(),"Solution diverged after "+to_str(iter)+" iterations");
  }
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
