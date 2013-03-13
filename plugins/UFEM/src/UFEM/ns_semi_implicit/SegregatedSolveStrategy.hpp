// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SegregatedSolveStrategy_hpp
#define cf3_UFEM_SegregatedSolveStrategy_hpp


#include <Teko_BlockedMappingStrategy.hpp>
#include <Teko_Utilities.hpp>

#include <Thyra_EpetraLinearOp.hpp>
#include <Thyra_VectorStdOps.hpp>

#include "math/VariablesDescriptor.hpp"
#include "math/LSS/SolutionStrategy.hpp"
#include "math/LSS/Trilinos/TekoBlockedOperator.hpp"

#include "../LibUFEM.hpp"

namespace cf3 {
namespace UFEM {

class UFEM_API SegregatedSolveStrategy : math::LSS::SolutionStrategy
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
  /// VariablesDescriptor that describes the blocking of the system
  Handle<math::VariablesDescriptor> m_variables_descriptor;

  Teuchos::RCP<Teko::Epetra::BlockedMappingStrategy> m_blocked_mapping;
  const Teuchos::RCP<Thyra::BlockedLinearOpBase<double> > m_blocked_thyra_op;
  Teuchos::RCP<const Thyra::LinearOpBase<double> > m_Auu;
  Teuchos::RCP<const Thyra::LinearOpBase<double> > m_Aup;
  Teuchos::RCP<const Thyra::LinearOpBase<double> > m_Apu;
  Teuchos::RCP<const Thyra::LinearOpBase<double> > m_App;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_SegregatedSolveStrategy_hpp
