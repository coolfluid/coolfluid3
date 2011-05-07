// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_ComputeRhsInCell_hpp
#define CF_Solver_Actions_ComputeRhsInCell_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "SFDM/LibSFDM.hpp"
#include "Mesh/CTable.hpp"
#include "Math/MatrixTypes.hpp"
#include "Mesh/CFieldView.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

  class Reconstruct;
  class Flux;

///////////////////////////////////////////////////////////////////////////////////////

/// @class ComputeRhsInCell
/// @brief Computes the RHS in one cell.
///
/// It is the workhorse of SFD Solver.

class SFDM_API ComputeRhsInCell : public Solver::Actions::CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeRhsInCell> Ptr;
  typedef boost::shared_ptr<ComputeRhsInCell const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeRhsInCell ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeRhsInCell() {};

  /// Get the class name
  static std::string type_name () { return "ComputeRhsInCell"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();
  void config_residual();

  void trigger_elements();
  
  RealRowVector to_row_vector(Mesh::CTable<Real>::ConstRow row) const ;
  RealMatrix    to_matrix(Mesh::CMultiStateFieldView::View data) const ;

private: // data
  
  boost::shared_ptr<Mesh::CMultiStateFieldView> m_solution;
  boost::shared_ptr<Mesh::CMultiStateFieldView> m_residual;

  boost::shared_ptr<Reconstruct> m_reconstruct_solution;
  boost::shared_ptr<Reconstruct> m_reconstruct_flux;

  boost::shared_ptr<Flux> m_flux;

};

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_ComputeRhsInCell_hpp
