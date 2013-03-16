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
#include "math/LSS/Trilinos/ThyraMultiVector.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"

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

  /// VariablesDescriptor that describes the blocking of the system
  Handle<math::VariablesDescriptor> m_variables_descriptor;

  Teuchos::RCP<Teko::Epetra::BlockedMappingStrategy> m_blocked_mapping;
  Teuchos::RCP< Thyra::PhysicallyBlockedLinearOpBase<Real> > m_blocked_thyra_op;

  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Auu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Aup;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_Apu;
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > m_App;

  Teuchos::RCP<Teko::InverseLibrary> m_inv_lib;
  Teuchos::RCP<Teko::InverseFactory> m_uu_inv_factory;

  Handle<math::LSS::TrilinosCrsMatrix> m_full_matrix;
  Handle<math::LSS::ThyraMultiVector> m_rhs;
  Handle<math::LSS::ThyraMultiVector> m_solution;

  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > m_blocked_rhs;
  Teuchos::RCP<Thyra::ProductMultiVectorBase<Real> > m_blocked_solution;
  
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_u_solution;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_p_solution;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_u_rhs;
  Teuchos::RCP<Thyra::MultiVectorBase<Real> > m_p_rhs;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_SegregatedSolveStrategy_hpp
