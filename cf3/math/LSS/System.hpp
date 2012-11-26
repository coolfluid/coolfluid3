// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_System_hpp
#define cf3_Math_LSS_System_hpp

////////////////////////////////////////////////////////////////////////////////////////////

//#include <boost/utility.hpp>

#include "math/LSS/LibLSS.hpp"
#include "common/Component.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "math/LSS/BlockAccumulator.hpp"
#include "math/LSS/Matrix.hpp"
#include "math/LSS/Vector.hpp"
#include "SolutionStrategy.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file System.hpp main header of the linear system solver interface.
  @brief This header collects all the headers needed for the linear system solver, also including configure-time present dependency specializations.
  @author Tamas Banyai

  The structure is organized into four main unit:
  * Vector: designed to wrap the right hand side and the solution
  * Matrix: obviously, the matrix
  * System: encapsulation of a matrix and two vectors
  * BlockAccumulator: a little class to collect "element-wise" data, in order to reduce cache misses

  Notation:
  * eq: size of the equation system (for example 2D incompressible navier stokes has neq=3, for solving p,u,v)
  * block: an nequation*nequation matrix
  * blockrow/blockcol: ith block in the row/column
  * row/col: ith actual row/column in the matrix (translates as irow=iblockrow*neq+ieq or the inverse iblockrow=irow/neq ieq=irow%neq)

  Include System.h in order to include everything related to linear system solver.
  In case the underlying dependency works with closed data, therefore the create function of the vector takes an LSS::Matrix as an argument.
  Also, the actual implementation of solve is in LSS::Matrix taking two LSS::Vector arguments as rhs and solution.
  For performance reasons, dynamic buld of a matrix is not allowed, If you change your matrix, collect your changes into commpattern + connectiviy and call create again.
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

class VariablesDescriptor;
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API System : public common::Component {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// name of the type
  static std::string type_name () { return "System"; }

  /// Default constructor
  System(const std::string& name);

  /// Setup sparsity structure
  /// @todo action for it
  void create(cf3::common::PE::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices);

  /// Create a blocked system, where the unknowns for each physical variable are stored together. Note that this only changes the internal ordering,
  /// the interface is not affected.
  void create_blocked(cf3::common::PE::CommPattern& cp, const VariablesDescriptor& vars, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices);

  /// Exchange to existing matrix and vectors
  /// @todo action for it
  void swap(const boost::shared_ptr<LSS::Matrix>& matrix, const boost::shared_ptr<LSS::Vector>& solution, const boost::shared_ptr<LSS::Vector>& rhs);

  /// Deallocate underlying data
  void destroy();

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name SOLVE THE SYSTEM
  //@{

  /// solving the system
  /// @todo action for it
  void solve();

  //@} END SOLVE THE SYSTEM

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  void set_values(const BlockAccumulator& values);

  /// Add a list of values
  void add_values(const BlockAccumulator& values);

  /// Add a list of values
  void get_values(BlockAccumulator& values);

  /// Apply dirichlet-type boundary conditions.
  /// When preserve_symmetry is true than blockrow*numequations+eq column is is zeroed by moving it to the right hand side (however this usually results in performance penalties).
  void dirichlet(const Uint iblockrow, const Uint ieq, const Real value, const bool preserve_symmetry=false);

  /// Applying periodicity by adding one line to another and dirichlet-style fixing it to
  /// Note that prerequisite for this is to work that the matrix sparsity should be compatible (same nonzero pattern for the two block rows).
  /// Note that only structural symmetry can be preserved (again, if sparsity input was symmetric).
  void periodicity (const Uint iblockrow_to, const Uint iblockrow_from);

  /// Set the diagonal
  void set_diagonal(const std::vector<Real>& diag);

  /// Add to the diagonal
  void add_diagonal(const std::vector<Real>& diag);

  /// Get the diagonal
  void get_diagonal(std::vector<Real>& diag);

  /// Reset Matrix
  void reset(Real reset_to=0.);

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to logstream
  void print(common::LogStream& stream);

  /// Print to wherever
  void print(std::ostream& stream);

  /// Print to file given by filename
  void print(const std::string& filename);

  /// Accessor to matrix
  Handle<LSS::Matrix> matrix() { return m_mat; }

  /// Accessor to right hand side
  Handle<LSS::Vector> rhs() { return m_rhs; }

  /// Accessor to solution
  Handle<LSS::Vector> solution() { return m_sol; }

  /// Accessor to the solution strategy
  Handle<LSS::SolutionStrategy> solution_strategy() { return m_solution_strategy; }

  /// Accessor to the state of create
  const bool is_created();

  /// Accessor to string option describing the type of the solver
  const std::string solvertype() { return m_mat->solvertype(); }

  //@} END MISCELLANEOUS

  /// @name SIGNALS
  //@{

  /// Signal to write the system to disk as a tecplot file, for debugging purposes.
  void signal_print(common::SignalArgs& args);

  //@}

private:

  void signature_print(common::SignalArgs& args);

  /// shared_ptr to system matrix
  Handle<LSS::Matrix> m_mat;

  /// shared_ptr to solution vectoe
  Handle<LSS::Vector> m_sol;

  /// shared_ptr to right hand side vector
  Handle<LSS::Vector> m_rhs;

  /// Strategy for the solution
  Handle<LSS::SolutionStrategy> m_solution_strategy;

}; // end of class System

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_System_hpp
