// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <Thyra_DefaultSpmdMultiVector.hpp>
#include <Thyra_VectorStdOps.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "SegregatedSolveStrategy.hpp"

namespace cf3 {
namespace UFEM {

common::ComponentBuilder<SegregatedSolveStrategy, math::LSS::SolutionStrategy, LibUFEM> SegregatedSolveStrategy_builder;

SegregatedSolveStrategy::SegregatedSolveStrategy ( const std::string& name ) : SolutionStrategy(name)
{
  options().add("variables_descriptor", m_variables_descriptor)
    .pretty_name("Variables Descriptor")
    .description("Variables descriptor for the layout of the system")
    .link_to(&m_variables_descriptor)
    .attach_trigger(boost::bind(&SegregatedSolveStrategy::trigger_variables_descriptor, this));    
}

SegregatedSolveStrategy::~SegregatedSolveStrategy()
{
}


void SegregatedSolveStrategy::solve()
{
  if(is_null(m_variables_descriptor))
    throw common::SetupError(FromHere(), "Option variables_descriptor is not set for " + uri().string());
  
  m_blocked_rhs = get_blocked_vector(m_rhs, m_blocked_thyra_op->domain());
  m_u_rhs = Teko::getBlock(m_u_idx, m_blocked_rhs);
  m_p_rhs = Teko::getBlock(m_p_idx, m_blocked_rhs);

  m_blocked_solution = get_blocked_vector(m_solution, m_blocked_thyra_op->range());
  m_u_solution = Teko::getBlock(m_u_idx, m_blocked_solution);
  m_p_solution = Teko::getBlock(m_p_idx, m_blocked_solution);

  m_blocked_mapping->rebuildBlockedThyraOp(m_full_matrix->epetra_matrix(), m_blocked_thyra_op);
  
  Teuchos::RCP<Thyra::LinearOpBase<Real> > Auu_inv = Teko::buildInverse(*m_uu_inv_factory, m_Auu);
  Thyra::apply(*Auu_inv, Thyra::NOTRANS, *m_u_rhs, m_u_solution.ptr());
  
  Handle<math::LSS::TrilinosVector> solution(m_solution);
  cf3_assert(is_not_null(solution));
  
  m_blocked_mapping->copyThyraIntoEpetra(m_blocked_solution, *solution->epetra_vector());
}

void SegregatedSolveStrategy::set_matrix ( const Handle< math::LSS::Matrix >& matrix )
{
  m_full_matrix = Handle<math::LSS::TrilinosCrsMatrix>(matrix);
  if(is_null(m_full_matrix))
    throw common::SetupError(FromHere(), "Error applying SegregatedSolveStrategy: matrix " + matrix->uri().string() + " is not a math::LSS::TrilinosCrsMatrix");

  trigger_variables_descriptor();
}

void SegregatedSolveStrategy::set_rhs ( const Handle< math::LSS::Vector >& rhs )
{
  m_rhs = Handle<math::LSS::ThyraMultiVector>(rhs);
}

void SegregatedSolveStrategy::set_solution ( const Handle< math::LSS::Vector >& solution )
{
  m_solution = Handle<math::LSS::ThyraMultiVector>(solution);
}

Real SegregatedSolveStrategy::compute_residual()
{
  return 0;
}

void SegregatedSolveStrategy::trigger_variables_descriptor()
{
  if(is_null(m_full_matrix) || is_null(m_rhs) || is_null(m_solution))
    return;

  if(is_null(m_variables_descriptor))
    return;

  const Teuchos::RCP<Epetra_CrsMatrix const> crs_matrix = m_full_matrix->epetra_matrix();
  const math::VariablesDescriptor& descriptor = *m_variables_descriptor;

  // Find out what the locations of the pressure and velocity variables are
  m_p_idx = descriptor.var_number("Pressure");
  m_u_idx = descriptor.var_number("Velocity");
  cf3_assert(m_p_idx == 0 || m_p_idx == 1);
  cf3_assert(m_u_idx == 0 || m_u_idx == 1);
  cf3_assert(descriptor.nb_vars() == 2);
  cf3_assert(descriptor.offset(m_u_idx) == m_u_idx);

  // Get the different blocks from the full matrix
  std::vector< std::vector<int> > vars;
  m_full_matrix->blocked_var_gids(descriptor, vars);
  m_blocked_mapping = Teuchos::rcp(new Teko::Epetra::BlockedMappingStrategy(vars, Teuchos::rcpFromRef(crs_matrix->OperatorDomainMap()), crs_matrix->Comm()));
  Teko::Epetra::BlockedMappingStrategy& blocked_mapping = *m_blocked_mapping;

  m_blocked_thyra_op = Teuchos::rcp_dynamic_cast< Thyra::PhysicallyBlockedLinearOpBase<Real> >(m_blocked_mapping->buildBlockedThyraOp(crs_matrix));
  m_Auu = Teko::getBlock(m_u_idx, m_u_idx, m_blocked_thyra_op);
  m_Aup = Teko::getBlock(m_u_idx, m_p_idx, m_blocked_thyra_op);
  m_Apu = Teko::getBlock(m_p_idx, m_u_idx, m_blocked_thyra_op);
  m_App = Teko::getBlock(m_p_idx, m_p_idx, m_blocked_thyra_op);

  m_inv_lib = Teko::InverseLibrary::buildFromStratimikos();
  m_uu_inv_factory = m_inv_lib->getInverseFactory("Amesos");
}

Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > SegregatedSolveStrategy::get_blocked_vector(const Handle<math::LSS::ThyraMultiVector> vec, const Teuchos::RCP< const Thyra::VectorSpaceBase<Real> >& space)
{
  Handle<math::LSS::TrilinosVector> t_vec(vec);
  if(is_null(t_vec))
  {
    throw common::SetupError(FromHere(), "UFEM::SegregatedSolveStrategy requires all vectors to be of type math::LSS::TrilinosVector");
  }

  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > result = Teuchos::rcp_dynamic_cast< Thyra::ProductMultiVectorBase<Real> >(Thyra::createMembers(space, 1));
  cf3_assert(!Teuchos::is_null(result));
  m_blocked_mapping->copyEpetraIntoThyra(*t_vec->epetra_vector(), result.ptr());
  return result;
}
  
} // UFEM
} // cf3
