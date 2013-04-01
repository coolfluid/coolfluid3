// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SegregatedSolveStrategy_hpp
#define cf3_UFEM_SegregatedSolveStrategy_hpp


#include <Teko_BlockedMappingStrategy.hpp>
#include <Teko_InverseFactory.hpp>
#include <Teko_InverseLibrary.hpp>
#include <Teko_Utilities.hpp>

#include <Thyra_EpetraLinearOp.hpp>
#include <Thyra_VectorStdOps.hpp>

#include "math/VariablesDescriptor.hpp"
#include "math/LSS/SolutionStrategy.hpp"
#include "math/LSS/Trilinos/ParameterList.hpp"
#include "math/LSS/Trilinos/ThyraMultiVector.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/System.hpp"

#include "solver/Time.hpp"

#include "../LibUFEM.hpp"

namespace cf3 {
namespace UFEM {

class UFEM_API SegregatedSolveStrategy : public math::LSS::SolutionStrategy
{
public:
  SegregatedSolveStrategy(const std::string& name);
  virtual ~SegregatedSolveStrategy();
  
  static std::string type_name () { return "SegregatedSolveStrategy"; }
  
  virtual void solve();
  virtual void set_matrix ( const Handle< math::LSS::Matrix >& matrix );
  virtual void set_rhs ( const Handle< math::LSS::Vector >& rhs );
  virtual void set_solution ( const Handle< math::LSS::Vector >& solution );
  virtual Real compute_residual();
  
private:
  void trigger_variables_descriptor();
  Teuchos::RCP< Thyra::ProductMultiVectorBase< Real > > get_blocked_vector( const cf3::Handle< cf3::math::LSS::ThyraMultiVector > vec, const Teuchos::RCP< const Thyra::VectorSpaceBase< cf3::Real > >& space );

  Uint m_p_idx;
  Uint m_u_idx;
  Uint m_p_offset;

  /// VariablesDescriptor that describes the blocking of the system
  Handle<math::VariablesDescriptor> m_variables_descriptor;

  Teuchos::RCP<Teko::Epetra::BlockedMappingStrategy> m_blocked_mapping;
  Teuchos::RCP< Thyra::PhysicallyBlockedLinearOpBase<Real> > m_blocked_system_op;
  Teuchos::RCP< Thyra::PhysicallyBlockedLinearOpBase<Real> > m_blocked_rhs_op;
  Teuchos::RCP< Thyra::PhysicallyBlockedLinearOpBase<Real> > m_blocked_t_op;

  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Muu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Mup;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Mpu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Mpp;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Tpu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Tuu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Auu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Aup;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Apu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_App;

  Teuchos::RCP<Teko::InverseLibrary> m_inv_lib;
  Teuchos::RCP<Teko::InverseFactory> m_uu_inv_factory;
  Stratimikos::DefaultLinearSolverBuilder m_p_linear_solver_builder;
  Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<Real> > m_p_lows_factory;
  Teuchos::RCP<Thyra::LinearOpWithSolveBase<Real> > m_p_lows;

  Handle<math::LSS::TrilinosCrsMatrix> m_full_matrix;
  Handle<math::LSS::ThyraMultiVector> m_rhs;
  Handle<math::LSS::ThyraMultiVector> m_solution;
  // Copy of the initial RHS, which contains the boundary conditions
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_bc_rhs;

  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > m_blocked_rhs;
  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > m_blocked_solution;
  
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_u;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_p;
  Teuchos::RCP<Thyra::VectorBase<Real> > m_u_rhs;
  Teuchos::RCP<Thyra::VectorBase<Real> > m_p_rhs;
  Teuchos::RCP<Thyra::VectorBase<Real> > m_u_rhs_mask;
  Teuchos::RCP<Thyra::VectorBase<Real> > m_u_bc;
  Teuchos::RCP<Thyra::VectorBase<Real> > m_p_rhs_mask;
  Teuchos::RCP<Thyra::VectorBase<Real> > m_p_bc;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_delta_a_star;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_a;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_delta_p;
  
  Handle<math::LSS::System> m_rhs_system;
  Handle<math::LSS::System> m_t_system;
  Handle<math::LSS::ParameterList> m_pressure_parameters;
  Teuchos::RCP<Teuchos::ParameterList> m_pressure_parameter_list;
  Handle<math::LSS::ParameterList> m_velocity_parameters;
  Teuchos::RCP<Teuchos::ParameterList> m_velocity_parameter_list;
  
  Handle<solver::Time> m_time;

  Uint m_nb_iterations;
  std::vector< std::pair<Uint, Uint> > m_dirichlet_nodes;

  Real m_theta;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_SegregatedSolveStrategy_hpp
