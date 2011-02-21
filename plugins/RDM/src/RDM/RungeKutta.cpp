// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
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
  CIterativeSolver ( name ),
  m_cfl(1.0)
{
  // options
  m_properties.add_option< OptionT<Real> > ("CFL", "Courant Number", m_cfl)
    ->mark_basic()
    ->link_to(&m_cfl)
    ->add_tag("cfl");

  // properties

  properties()["brief"] = std::string("Iterative RDM component");
  properties()["description"] = std::string("Forward Euler Time Stepper");

  m_properties["Domain"].as_option().attach_trigger ( boost::bind ( &RungeKutta::config_domain,   this ) );

  // signals

  this->regist_signal ( "solve" , "Solve", "Solve" )->connect ( boost::bind ( &RungeKutta::solve, this ) );

  m_solution_field =     create_static_component<CLink>( "solution" );
  m_residual_field =     create_static_component<CLink>( "residual" );
  m_update_coeff_field = create_static_component<CLink>( "update_coeff" );

}

////////////////////////////////////////////////////////////////////////////////

RungeKutta::~RungeKutta()
{
}

////////////////////////////////////////////////////////////////////////////////

void RungeKutta::config_domain()
{
  CDomain::Ptr domain = look_component<CDomain>( property("Domain").value<URI>() );
  if( is_null(domain) )
    throw InvalidURI( FromHere(), "Path does not point to Domain");

  CMesh::Ptr mesh = find_component_ptr_recursively<CMesh>( *domain );
  if (is_not_null(mesh))
  {
    std::string solution_tag("solution");
    CField2::Ptr solution = find_component_ptr_with_tag<CField2>(*mesh, solution_tag);
    if ( is_null(solution) )
    {
      solution = mesh->create_field2("solution","PointBased","u[1]").as_type<CField2>();
      solution->add_tag(solution_tag);
    }
    m_solution_field->link_to(solution);

    CFinfo << " - solution field : " << solution->full_path().string() << CFendl;

    std::string residual_tag("residual");
    CField2::Ptr residual = find_component_ptr_with_tag<CField2>(*mesh, residual_tag);
    if ( is_null(residual) )
    {
      residual = mesh->create_field2("residual","PointBased","residual[1]").as_type<CField2>();
      residual->add_tag(residual_tag);
    }
    m_residual_field->link_to(residual);

    CFinfo << " - residual field : " << residual->full_path().string() << CFendl;

    std::string update_coeff_tag("update_coeff");
    CField2::Ptr update_coeff = find_component_ptr_with_tag<CField2>(*mesh, update_coeff_tag);
    if ( is_null(update_coeff) )
    {
      update_coeff = mesh->create_field2("update_coeff","PointBased","update_coeff[1]").as_type<CField2>();
      update_coeff->add_tag(update_coeff_tag);
    }
    m_update_coeff_field->link_to(update_coeff);

    CFinfo << " - update_coeff field : " << update_coeff->full_path().string() << CFendl;

    // configure DM mesh after fields are created since DM will look for them
    // on the attached configuration trigger
    discretization_method().configure_property( "Mesh" , mesh->full_path() );
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

  /*
  CField2::Ptr solution     = m_solution_field->follow()->as_type<CField2>();
  CField2::Ptr residual     = m_residual_field->follow()->as_type<CField2>();
  CField2::Ptr update_coeff = m_update_coeff_field->follow()->as_type<CField2>();
  */


  CTable<Real>& solution = m_solution_field->follow()->as_type<CField2>()->data();
  CTable<Real>& residual = m_residual_field->follow()->as_type<CField2>()->data();
  CTable<Real>& update_coeff = m_update_coeff_field->follow()->as_type<CField2>()->data();

//  CFinfo << "DATA TABLE SIZES:" << CFendl;
//  CFinfo << "solution: " << solution.size() << " x " << solution.row_size() << CFendl;
//  CFinfo << "residual: " << residual.size() << " x " << residual.row_size() << CFendl;
//  CFinfo << "update_coeff: " << update_coeff.size() << " x " << update_coeff.row_size() << CFendl;

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

//    CFinfo << " --  cleaning residual field" << CFendl;

    // set update coefficient and residual to zero
    // Set the field data of the source field



   /*
    boost_foreach (CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_residual_field->follow(), "node_data"))
    {
      Uint size = node_data.size();
      for (Uint i=0; i<size; ++i)
        node_data[i][0]=0.;
    }

//    CFinfo << " --  cleaning update coeff" << CFendl;
    boost_foreach (CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(*m_update_coeff_field->follow(),"node_data"))
    {
      Uint size = node_data.size();
      for (Uint i=0; i<size; ++i)
        node_data[i][0]=0.;
    }
    */

    Uint size = residual.size();
    for (Uint i=0; i<size; ++i)
      residual[i][0]=0;

    size = update_coeff.size();
    for (Uint i=0; i<size; ++i)
      update_coeff[i][0]=0;





    // compute RHS
//    CFinfo << " --  computing the rhs" << CFendl;
    discretization_method().compute_rhs();

    // explicit update
//    CFinfo << " --  updating solution" << CFendl;
    const Uint nbdofs = solution.size();
    for (Uint i=0; i< nbdofs; ++i)
      solution[i][0] += - ( m_cfl / update_coeff[i][0] ) * residual[i][0];

//    CFinfo << " --  computing the norm" << CFendl;
    Real rhs_L2 = 0.;
    for (Uint i=0; i< nbdofs; ++i)
      rhs_L2 += residual[i][0] * residual[i][0];
    rhs_L2 = sqrt(rhs_L2)/nbdofs;

    // output convergence info
    CFinfo << "Iter [" << std::setw(4) << iter << "] L2(rhs) [" << std::setw(12) << rhs_L2 << "]" << CFendl;
//    if ( is_nan(rhs_L2) || is_inf(rhs_L2) )
//      throw FailedToConverge(FromHere(),"Solution diverged after "+to_str(iter)+" iterations");
  }
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
