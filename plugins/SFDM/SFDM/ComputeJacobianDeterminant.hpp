// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_ComputeJacobianDeterminant_hpp
#define CF_Solver_Actions_ComputeJacobianDeterminant_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "SFDM/LibSFDM.hpp"
#include "Mesh/CTable.hpp"
#include "Math/MatrixTypes.hpp"
#include "Mesh/CFieldView.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

///////////////////////////////////////////////////////////////////////////////////////

/// @class ComputeJacobianDeterminant
/// @brief Computes the Jacobian determinant in every solution point in one cell.
class SFDM_API ComputeJacobianDeterminant : public Solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeJacobianDeterminant> Ptr;
  typedef boost::shared_ptr<ComputeJacobianDeterminant const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeJacobianDeterminant ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeJacobianDeterminant() {};

  /// Get the class name
  static std::string type_name () { return "ComputeJacobianDeterminant"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_jacobian_determinant();

  void trigger_elements();
  
private: // data
  
  boost::shared_ptr<Mesh::CMultiStateFieldView> m_jacobian_determinant;
};

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_ComputeJacobianDeterminant_hpp
