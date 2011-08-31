// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_TrilinosMatrix_hpp
#define CF_Math_LSS_TrilinosMatrix_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <Epetra_MpiComm.h>
#include <Epetra_FEVbrMatrix.h>
#include <Teuchos_RCP.hpp>

#include "Math/LibMath.hpp"
#include "Math/LSS/BlockAccumulator.hpp"
#include "Math/LSS/Vector.hpp"
#include "Math/LSS/Matrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosMatrix.hpp definition of LSS::TrilinosMatrix
  @author Tamas Banyai

  It is based on Trilinos's FEVbrMatrix.
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class Math_API TrilinosMatrix : public LSS::Matrix {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<TrilinosMatrix> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<TrilinosMatrix const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "TrilinosMatrix"; }

  /// Accessor to solver type
  const std::string solvertype() { return "Trilinos"; }

  /// Accessor to the flag if matrix, solution and rhs are tied together or not
  const bool compatible(const LSS::Vector::Ptr solution, const LSS::Vector::Ptr rhs) { return true; };

  /// Default constructor
  TrilinosMatrix(const std::string& name);

  /// Setup sparsity structure
  void create(CF::Common::Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs);

  /// Deallocate underlying data
  void destroy();

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  void set_value(const Uint icol, const Uint irow, const Real value);

  /// Add value at given location in the matrix
  void add_value(const Uint icol, const Uint irow, const Real value);

  /// Get value at given location in the matrix
  void get_value(const Uint icol, const Uint irow, Real& value);

  //@} END INDIVIDUAL ACCESS

  /// @name SOLVE THE SYSTEM
  //@{

  /// The holy solve, for solving the m_mat*m_sol=m_rhs problem.
  /// We bow on our knees before your greatness.
  void solve(LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs);

  //@} END SOLVE THE SYSTEM

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  void set_values(const BlockAccumulator& values);

  /// Add a list of values
  /// local ibdices
  /// eigen, templatization on top level
  void add_values(const BlockAccumulator& values);

  /// Add a list of values
  void get_values(BlockAccumulator& values);

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  void set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval);

  /// Get a column and replace it to zero (dirichlet-type boundaries, when trying to preserve symmetry)
  /// Note that sparsity info is lost, values will contain zeros where no matrix entry is present
  void get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values);

  /// Add one line to another and tie to it via dirichlet-style (applying periodicity)
  void tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from);

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

  /// Print to wherever
  void print(Common::LogStream& stream);

  /// Print to wherever
  void print(std::ostream& stream);

  /// Print to file given by filename
  void print(const std::string& filename);

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; }

  /// Accessor to the number of equations
  const Uint neq() { cf_assert(m_is_created); return m_neq; }

  /// Accessor to the number of block rows
  const Uint blockrow_size() {  cf_assert(m_is_created); return m_blockrow_size; }

  /// Accessor to the number of block columns
  const Uint blockcol_size() {  cf_assert(m_is_created); return m_blockcol_size; }

  //@} END MISCELLANEOUS

private:

  /// teuchos style smart pointer wrapping an epetra fevbrmatrix
  Teuchos::RCP<Epetra_FEVbrMatrix> m_mat;

  /// epetra mpi environment
  Epetra_MpiComm m_comm;

  /// state of creation
  bool m_is_created;

  /// number of equations
  Uint m_neq;

  /// number of block rows
  Uint m_blockrow_size;

  /// number of block columns
  Uint m_blockcol_size;


}; // end of class Matrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_TrilinosMatrix_hpp
