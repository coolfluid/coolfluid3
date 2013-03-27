// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <Thyra_DefaultScaledAdjointLinearOp.hpp>
#include <Thyra_DefaultSpmdMultiVector.hpp>
#include <Thyra_VectorStdOps.hpp>
#include <Thyra_MultiVectorStdOps.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "math/LSS/Trilinos/ParameterListDefaults.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "SegregatedSolveStrategy.hpp"

namespace cf3 {
namespace UFEM {

void write_mat(const Teuchos::RCP<const Thyra::LinearOpBase<Real> >& mat, const std::string& filename)
{
// Teuchos::RCP<std::ofstream> mat_out = Teuchos::rcp(new  std::ofstream(filename.c_str(), std::ios::out));
// Teuchos::RCP<Teuchos::FancyOStream> mat_fancy_out = Teuchos::fancyOStream(mat_out);
// Thyra::describeLinearOp(*mat, *mat_fancy_out, Teuchos::VERB_EXTREME);
}
  
common::ComponentBuilder<SegregatedSolveStrategy, math::LSS::SolutionStrategy, LibUFEM> SegregatedSolveStrategy_builder;

SegregatedSolveStrategy::SegregatedSolveStrategy ( const std::string& name ) :
  SolutionStrategy(name),
  m_nb_iterations(2),
  m_theta(1.)
{
  options().add("variables_descriptor", m_variables_descriptor)
    .pretty_name("Variables Descriptor")
    .description("Variables descriptor for the layout of the system")
    .link_to(&m_variables_descriptor)
    .attach_trigger(boost::bind(&SegregatedSolveStrategy::trigger_variables_descriptor, this));
    
  options().add("rhs_system", m_rhs_system)
    .pretty_name("RHS system")
    .description("Matrix used in RHS assembly (stiffness part)")
    .link_to(&m_rhs_system);

  options().add("t_system", m_t_system)
    .pretty_name("t system")
    .description("Matrix used in t assembly (mass matrix part)")
    .link_to(&m_t_system);
    
  options().add("time", m_time).pretty_name("Time").description("Time keeping component").link_to(&m_time);

  options().add("nb_iterations", m_nb_iterations)
    .pretty_name("Nb Iterations")
    .description("Number of iterations to perform in the inner loop")
    .link_to(&m_nb_iterations);

  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta scheme parameter")
    .link_to(&m_theta);

  m_parameter_list = Teuchos::rcp(new Teuchos::ParameterList(Stratimikos::DefaultLinearSolverBuilder().getValidParameters()->sublist("Linear Solver Types")));

  m_parameters = create_component<math::LSS::ParameterList>("Parameters");
  m_parameters->mark_basic();

  // loop over all entries in solver list
  for(Teuchos::ParameterList::ConstIterator itr = m_parameter_list->begin(); itr!=m_parameter_list->end(); ++itr)
  {
    itr->second.getValue(static_cast<Teuchos::ParameterList*>(0)).set("Type", itr->first);
  }

  m_parameters->set_parameter_list(*m_parameter_list);
  m_parameter_list->print();
}

SegregatedSolveStrategy::~SegregatedSolveStrategy()
{
}

void SegregatedSolveStrategy::solve()
{
  if(is_null(m_variables_descriptor))
    throw common::SetupError(FromHere(), "Option variables_descriptor is not set for " + uri().string());
  
  if(is_null(m_rhs_system))
    throw common::SetupError(FromHere(), "Option rhs_system is not set for " + uri().string());
  
  if(is_null(m_time))
    throw common::SetupError(FromHere(), "Option time is not set for " + uri().string());
  
  Handle<math::LSS::ThyraOperator> rhs_thyra(m_rhs_system->matrix());
  if(is_null(rhs_thyra))
    throw common::SetupError(FromHere(), "Option rhs_system is not a math::LSS::ThyraOperator for " + uri().string() + " (provided was: " + m_rhs_system->matrix()->derived_type_name() + ")");

  Handle<math::LSS::TrilinosVector> solution(m_solution);
  cf3_assert(is_not_null(solution));
  
  Handle<math::LSS::TrilinosVector> rhs_trilvec(m_rhs);
  cf3_assert(is_not_null(rhs_trilvec));
  math::LSS::TrilinosVector& rhs = *rhs_trilvec;
  const Uint nb_nodes = rhs.blockrow_size();
  
  CFdebug << "Begin SegregatedSolve" << CFendl;

  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > blocked_rhs_mask;
  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > blocked_bc;
  blocked_bc = get_blocked_vector(m_rhs, m_blocked_system_op->domain());
  Thyra::assign(Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_u_bc.ptr()), *Teko::getBlock(m_u_idx, blocked_bc));
  Thyra::assign(Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_p_bc.ptr()), *Teko::getBlock(m_p_idx, blocked_bc));
  
  // Build a mask to be able to keep the dirichlet BCs
  if(m_dirichlet_nodes.empty())
  {
    m_full_matrix->get_dirichlet_nodes(m_dirichlet_nodes);
    
    const Uint nb_bc = m_dirichlet_nodes.size();
    rhs.reset(1.);
    for(Uint i = 0; i != nb_bc; ++i)
      rhs.set_value(m_dirichlet_nodes[i].first, m_dirichlet_nodes[i].second, 0.);
    
    blocked_rhs_mask = get_blocked_vector(m_rhs, m_blocked_system_op->domain());
    Thyra::assign(Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_u_rhs_mask.ptr()), *Teko::getBlock(m_u_idx, blocked_rhs_mask));
    Thyra::assign(Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_p_rhs_mask.ptr()), *Teko::getBlock(m_p_idx, blocked_rhs_mask));
  }
  
  // Rebuild blocked ops
  m_blocked_mapping->rebuildBlockedThyraOp(m_full_matrix->epetra_matrix(), m_blocked_system_op);
  Handle<math::LSS::TrilinosCrsMatrix> crs_t_mat(m_t_system->matrix());
  m_blocked_mapping->rebuildBlockedThyraOp(crs_t_mat->epetra_matrix(), m_blocked_t_op);
  Handle<math::LSS::TrilinosCrsMatrix> crs_rhs_mat(m_rhs_system->matrix());
  m_blocked_mapping->rebuildBlockedThyraOp(crs_rhs_mat->epetra_matrix(), m_blocked_rhs_op);

  // Loop initialization
  Thyra::assign(m_a.ptr(), 0.);
  Thyra::assign(m_delta_p.ptr(), 0.);

  m_blocked_solution = get_blocked_vector(m_solution, m_blocked_system_op->range());
  m_u = Teko::getBlock(m_u_idx, m_blocked_solution);
  m_p = Teko::getBlock(m_p_idx, m_blocked_solution);

  //m_blocked_rhs = get_blocked_vector(m_rhs, m_blocked_system_op->domain());
  //Thyra::assign(Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_u_rhs.ptr()), *Teko::getBlock(m_u_idx, m_blocked_rhs));
  //Thyra::assign(Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_p_rhs.ptr()), *Teko::getBlock(m_p_idx, m_blocked_rhs));

  for(Uint i = 0; i != m_nb_iterations; ++i)
  {
    // Velocity RHS
    Thyra::apply(*m_Auu, Thyra::NOTRANS, *m_u, m_u_rhs.ptr(), -1.);
    Thyra::apply(*m_Aup, Thyra::NOTRANS, *m_p, m_u_rhs.ptr(), -1., 1.);
    // Velocity system a RHS terms
    Thyra::apply(*m_Muu, Thyra::NOTRANS, *m_a, m_u_rhs.ptr(), -1., 1.);
    // Apply RHS matrix to current a vector
    Thyra::apply(*m_Auu, Thyra::NOTRANS, *m_a, m_u_rhs.ptr(), m_time->dt(), 1.);
    // Velocity system BC
    //Thyra::ele_wise_prod_update(1., *m_u_rhs_mask, m_u_rhs.ptr());
    //Thyra::update(1., *Teuchos::rcp_static_cast< Thyra::MultiVectorBase<Real> >(m_u_bc), Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_u_rhs.ptr()));

    std::vector<Real> da_norms(m_delta_a_star->domain()->dim());
    std::vector<Real> da_star_norms(m_delta_a_star->domain()->dim());
    std::vector<Real> dp_norms(m_delta_p->domain()->dim());

    // Velocity system
    Teuchos::RCP<Thyra::LinearOpBase<Real> const> Muu_inv = Teko::buildInverse(*m_uu_inv_factory, m_Muu);
    Thyra::apply(*Muu_inv, Thyra::NOTRANS, *m_u_rhs, m_delta_a_star.ptr());
    Thyra::norms(*m_delta_a_star, Teuchos::arrayViewFromVector(da_star_norms));
    //m_delta_a_star->describe(*Teuchos::VerboseObjectBase::getDefaultOStream(), Teuchos::VERB_EXTREME);
    
    // Pressure system
    Thyra::apply(*m_Apu, Thyra::NOTRANS, *m_u, m_p_rhs.ptr(), 1.);
    Thyra::apply(*m_App, Thyra::NOTRANS, *m_p, m_p_rhs.ptr(), 1., 1.);
    // Add in RHS delta a* terms
    Thyra::apply(*m_Apu, Thyra::NOTRANS, *m_delta_a_star, m_p_rhs.ptr(), m_time->dt(), 1.);
    Thyra::apply(*m_Tpu, Thyra::NOTRANS, *m_delta_a_star, m_p_rhs.ptr(), 1., 1.);
    Thyra::apply(*m_Tpu, Thyra::NOTRANS, *m_a, m_p_rhs.ptr(), 1., 1.); // RHS a term
    // Velocity system BC
    //Thyra::ele_wise_prod_update(1., *m_p_rhs_mask, m_p_rhs.ptr());
    //Thyra::update(-1., *Teuchos::rcp_static_cast< Thyra::MultiVectorBase<Real> >(m_p_bc), Teuchos::ptr_static_cast< Thyra::MultiVectorBase<Real> >(m_p_rhs.ptr()));

    // Assemble the matrix operator
    Teuchos::RCP<Thyra::LinearOpBase<Real> const> p_mat =  Thyra::subtract(Thyra::multiply(m_Mpu, Muu_inv, m_Aup), m_App);
    Teuchos::RCP<Thyra::LinearOpBase<Real> const> p_inv = Teko::buildInverse(*m_pp_inv_factory, p_mat);
    std::cout << m_pp_inv_factory->toString() << std::endl;
    Thyra::apply(*p_inv, Thyra::NOTRANS, *m_p_rhs, m_delta_p.ptr());

    // Compute new delta a (stored in m_delta_a_star)
    Thyra::apply(*Thyra::multiply(Muu_inv, m_Mup), Thyra::NOTRANS, *m_delta_p, m_delta_a_star.ptr(), -1., 1.);
    // Compute a
    Thyra::update(1., *m_delta_a_star, m_a.ptr());
    // Compute u
    Thyra::update(m_time->dt(), *m_delta_a_star, m_u.ptr());
    // Compute p
    Thyra::assign(m_p.ptr(), 0.);
    Thyra::update(1., *m_delta_p, m_p.ptr());

    Thyra::norms(*m_delta_p, Teuchos::arrayViewFromVector(dp_norms));
    Thyra::norms(*m_delta_a_star, Teuchos::arrayViewFromVector(da_norms));

    CFdebug << "  Iteration " << i << " norms: delta_a_star: " << da_star_norms[0] << ", delta_a: " << da_norms[0] << ", delta p: " << dp_norms[0] << CFendl;
  }
  // Copy the final solution back to the original solution vector
  m_blocked_mapping->copyThyraIntoEpetra(m_blocked_solution, *solution->epetra_vector());
  CFdebug << "End SegregatedSolve" << CFendl;
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
  if(is_null(m_full_matrix) || is_null(m_rhs) || is_null(m_solution) || is_null(m_t_system) || is_null(m_rhs_system))
    return;

  if(is_null(m_variables_descriptor))
    return;

  const Teuchos::RCP<Epetra_CrsMatrix const> crs_matrix = m_full_matrix->epetra_matrix();
  const math::VariablesDescriptor& descriptor = *m_variables_descriptor;

  // Find out what the locations of the pressure and velocity variables are
  m_p_idx = descriptor.var_number("Pressure");
  m_u_idx = descriptor.var_number("Velocity");
  m_p_offset = descriptor.offset(m_p_idx);
  cf3_assert(m_p_idx == 0 || m_p_idx == 1);
  cf3_assert(m_u_idx == 0 || m_u_idx == 1);
  cf3_assert(descriptor.nb_vars() == 2);
  cf3_assert(descriptor.offset(m_u_idx) == m_u_idx);

  // Get the different blocks from the full matrix
  std::vector< std::vector<int> > vars;
  m_full_matrix->blocked_var_gids(descriptor, vars);
  m_blocked_mapping = Teuchos::rcp(new Teko::Epetra::BlockedMappingStrategy(vars, Teuchos::rcpFromRef(crs_matrix->OperatorDomainMap()), crs_matrix->Comm()));

  m_blocked_system_op = Teuchos::rcp_dynamic_cast< Thyra::PhysicallyBlockedLinearOpBase<Real> >(m_blocked_mapping->buildBlockedThyraOp(crs_matrix));
  m_Muu = Teko::getBlock(m_u_idx, m_u_idx, m_blocked_system_op);
  m_Mup = Teko::getBlock(m_u_idx, m_p_idx, m_blocked_system_op);
  m_Mpu = Teko::getBlock(m_p_idx, m_u_idx, m_blocked_system_op);
  m_Mpp = Teko::getBlock(m_p_idx, m_p_idx, m_blocked_system_op);

  Handle<math::LSS::TrilinosCrsMatrix> crs_t_mat(m_t_system->matrix());
  cf3_assert(is_not_null(crs_t_mat));
  m_blocked_t_op = Teuchos::rcp_dynamic_cast< Thyra::PhysicallyBlockedLinearOpBase<Real> >(m_blocked_mapping->buildBlockedThyraOp(crs_t_mat->epetra_matrix()));
  m_Tpu = Teko::getBlock(m_p_idx, m_u_idx, m_blocked_t_op);
  m_Tuu = Teko::getBlock(m_u_idx, m_u_idx, m_blocked_t_op);

  Handle<math::LSS::TrilinosCrsMatrix> crs_rhs_mat(m_rhs_system->matrix());
  cf3_assert(is_not_null(crs_rhs_mat));
  m_blocked_rhs_op = Teuchos::rcp_dynamic_cast< Thyra::PhysicallyBlockedLinearOpBase<Real> >(m_blocked_mapping->buildBlockedThyraOp(crs_rhs_mat->epetra_matrix()));
  m_Auu = Teko::getBlock(m_u_idx, m_u_idx, m_blocked_rhs_op);
  m_Aup = Teko::getBlock(m_u_idx, m_p_idx, m_blocked_rhs_op);
  m_Apu = Teko::getBlock(m_p_idx, m_u_idx, m_blocked_rhs_op);
  m_App = Teko::getBlock(m_p_idx, m_p_idx, m_blocked_rhs_op);

  m_inv_lib = Teko::InverseLibrary::buildFromParameterList(*m_parameter_list);

  m_uu_inv_factory = m_inv_lib->getInverseFactory("Amesos");
  m_pp_inv_factory = m_inv_lib->getInverseFactory("Belos");
  
  m_delta_a_star = Thyra::createMembers(m_Auu->range(), 1);
  m_a = Thyra::createMembers(m_Auu->range(), 1);
  m_delta_p = Thyra::createMembers(m_App->range(), 1);
  m_bc_rhs = Thyra::createMembers(m_full_matrix->thyra_operator()->domain(), 1);

  m_u_rhs = Thyra::createMember(m_Auu->range());
  m_u_bc = Thyra::createMember(m_Auu->range());
  m_u_rhs_mask = Thyra::createMember(m_Auu->range());
  
  m_p_rhs = Thyra::createMember(m_App->range());
  m_p_bc = Thyra::createMember(m_App->range());
  m_p_rhs_mask = Thyra::createMember(m_App->range());
  
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
