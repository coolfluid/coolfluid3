// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_TrilinosCrsMatrix_hpp
#define cf3_Math_LSS_TrilinosCrsMatrix_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <Epetra_MpiComm.h>
#include <Epetra_CrsMatrix.h>
#include <Teuchos_RCP.hpp>

#include "math/LSS/LibLSS.hpp"
#include "math/LSS/BlockAccumulator.hpp"
#include "math/LSS/Vector.hpp"
#include "math/LSS/Matrix.hpp"

#include "ThyraOperator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosCrsMatrix.hpp definition of LSS::TrilinosCrsMatrix
  @author Tamas Banyai

  It is based on Trilinos's FEVbrMatrix.
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API TrilinosCrsMatrix : public LSS::Matrix, public ThyraOperator {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// name of the type
  static std::string type_name () { return "TrilinosCrsMatrix"; }

  /// Accessor to solver type
  const std::string solvertype() { return "Trilinos"; }

  /// Accessor to the flag if matrix, solution and rhs are tied together or not
  const bool is_swappable(const LSS::Vector& solution, const LSS::Vector& rhs) { return true; }

  /// Default constructor
  TrilinosCrsMatrix(const std::string& name);

  /// Setup sparsity structure
  void create(cf3::common::PE::CommPattern& cp, const Uint neq, const std::vector<Uint>& node_connectivity, const std::vector<Uint>& starting_indices, LSS::Vector& solution, LSS::Vector& rhs);
  virtual void create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars, const std::vector< Uint >& node_connectivity, const std::vector< Uint >& starting_indices, Vector& solution, Vector& rhs);

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

  virtual void symmetric_dirichlet(const Uint blockrow, const Uint ieq, const Real value, Vector& rhs);

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
  void print(common::LogStream& stream);

  /// Print to wherever
  void print(std::ostream& stream);

  /// Print to file given by filename
  void print(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out );

  void print_native(ostream& stream);

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; }

  /// Accessor to the number of equations
  const Uint neq() { cf3_assert(m_is_created); return m_neq; }

  /// Accessor to the number of block rows
  const Uint blockrow_size() {  cf3_assert(m_is_created); return m_num_my_elements/neq(); }

  /// Accessor to the number of block columns
  const Uint blockcol_size() {  cf3_assert(m_is_created); return m_p2m.size()/neq(); }

  //@} END MISCELLANEOUS

  /// @name TEST ONLY
  //@{

  /// exports the matrix into big linear arrays
  /// @attention only for debug and utest purposes
  void debug_data(std::vector<Uint>& row_indices, std::vector<Uint>& col_indices, std::vector<Real>& values);

  //@} END TEST ONLY
  
  virtual Teuchos::RCP< const Thyra::LinearOpBase< Real > > thyra_operator() const;
  virtual Teuchos::RCP< Thyra::LinearOpBase< Real > > thyra_operator();

private:

  /// teuchos style smart pointer wrapping the matrix
  Teuchos::RCP<Epetra_CrsMatrix> m_mat;

  /// epetra mpi environment
  Epetra_MpiComm m_comm;

  /// state of creation
  bool m_is_created;

  /// number of equations
  Uint m_neq;

  /// number of local elements (rows)
  int m_num_my_elements;

  /// mapper array, maps from process local numbering to matrix local numbering (because ghost nodes need to be ordered to the back)
  std::vector<int> m_p2m;

  /// a helper array used in set/add/get_values to avoid frequent new+free combo
  std::vector<int> m_converted_indices;

  /// Copy of the connectivity data
  std::vector<int> m_node_connectivity, m_starting_indices;
}; // end of class Matrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_TrilinosCrsMatrix_hpp
