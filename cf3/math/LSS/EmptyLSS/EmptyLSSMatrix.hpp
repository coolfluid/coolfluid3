// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_EmptyLSSMatrix_hpp
#define cf3_Math_LSS_EmptyLSSMatrix_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "math/LSS/LibLSS.hpp"
#include "common/PE/CommPattern.hpp"
#include "math/LSS/Matrix.hpp"
#include "math/VariablesDescriptor.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file EmptyLSSMatrix.hpp implementation of LSS::EmptyLSSMatrix
  @author Tamas Banyai

  EmptyLSSMatrix is intended to use for testing purposes only.
  It acts like a fully operational linear solver but it does not solve and allocate any memory.
  // @todo turn it into a testing suite and throws for everything incorrect
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API EmptyLSSMatrix : public LSS::Matrix {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// name of the type
  static std::string type_name () { return "EmptyLSSMatrix"; }

  /// Accessor to solver type
  const std::string solvertype() { return "EmptyLSS"; }

  /// Accessor to the flag if matrix, solution and rhs are tied together or not
  virtual const bool is_swappable(const LSS::Vector& solution, const LSS::Vector& rhs) { return true; };

  /// Default constructor
  EmptyLSSMatrix(const std::string& name);

  /// Setup sparsity structure
  void create(cf3::common::PE::CommPattern& cp, const Uint neq, const std::vector<Uint>& node_connectivity, const std::vector<Uint>& starting_indices, LSS::Vector& solution, LSS::Vector& rhs)
  {
    destroy();
    m_neq=neq;
    m_blockrow_size=cp.gid()->size();
    m_blockcol_size=(*max_element(node_connectivity.begin(),node_connectivity.end(),std::less<Uint>()))+1;
    m_is_created=true;
  }

  void create_blocked(common::PE::CommPattern& cp, const VariablesDescriptor& vars, const std::vector< Uint >& node_connectivity, const std::vector< Uint >& starting_indices, Vector& solution, Vector& rhs)
  {
    destroy();
    m_neq=vars.size();
    m_blockrow_size=cp.gid()->size();
    m_blockcol_size=(*max_element(node_connectivity.begin(),node_connectivity.end(),std::less<Uint>()))+1;
    m_is_created=true;
  }

  /// Deallocate underlying data
  void destroy()
  {
    m_is_created=false;
    m_blockcol_size=0;
    m_blockrow_size=0;
    m_neq=0;
  }

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  void set_value(const Uint icol, const Uint irow, const Real value) { cf3_assert(m_is_created); }

  /// Add value at given location in the matrix
  void add_value(const Uint icol, const Uint irow, const Real value) { cf3_assert(m_is_created); }

  /// Get value at given location in the matrix
  void get_value(const Uint icol, const Uint irow, Real& value) {  cf3_assert(m_is_created); value=0.; }

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  void set_values(const BlockAccumulator& values) { cf3_assert(m_is_created); }

  /// Add a list of values
  /// local ibdices
  /// eigen, templatization on top level
  void add_values(const BlockAccumulator& values) { cf3_assert(m_is_created); }

  /// Add a list of values
  void get_values(BlockAccumulator& values) { cf3_assert(m_is_created); values.mat.setConstant(0.); }

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  void set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval) { cf3_assert(m_is_created); }

  /// Get a column and replace it to zero (dirichlet-type boundaries, when trying to preserve symmetry)
  /// Note that sparsity info is lost, values will contain zeros where no matrix entry is present
  void get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values) { cf3_assert(m_is_created); values.resize(m_blockcol_size*m_neq,0.); }

  void symmetric_dirichlet(const Uint blockrow, const Uint ieq, const Real value, Vector& rhs) { cf3_assert(m_is_created); }

  /// Add one line to another and tie to it via dirichlet-style (applying periodicity)
  void tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from) { cf3_assert(m_is_created); }

  /// Set the diagonal
  void set_diagonal(const std::vector<Real>& diag) { cf3_assert(m_is_created); cf3_assert(diag.size()==m_blockrow_size*m_neq); }

  /// Add to the diagonal
  void add_diagonal(const std::vector<Real>& diag) { cf3_assert(m_is_created); cf3_assert(diag.size()==m_blockrow_size*m_neq); }

  /// Get the diagonal
  void get_diagonal(std::vector<Real>& diag) {  cf3_assert(m_is_created); diag.assign(m_blockrow_size*m_neq,0.); }

  /// Reset Matrix
  void reset(Real reset_to=0.) { cf3_assert(m_is_created); }

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  void print(common::LogStream& stream) { cf3_assert(m_is_created); stream << "EmptyLSSMatrix::print of '" << name() << "'.\n"; }

  /// Print to wherever
  void print(std::ostream& stream) { cf3_assert(m_is_created); stream << "EmptyLSSMatrix::print of '" << name() << "'.\n"; }

  /// Print to file given by filename
  void print(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out ) { cf3_assert(m_is_created); }

  void print_native(std::ostream& stream) {}

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; }

  /// Accessor to the number of equations
  const Uint neq() {  cf3_assert(m_is_created); return m_neq; }

  /// Accessor to the number of block rows
  const Uint blockrow_size() {  cf3_assert(m_is_created); return m_blockrow_size; }

  /// Accessor to the number of block columns
  const Uint blockcol_size() {  cf3_assert(m_is_created); return m_blockcol_size; }

  //@} END MISCELLANEOUS

  /// @name TEST ONLY
  //@{

  /// exports the matrix into big linear arrays
  /// @attention only for debug and utest purposes
  virtual void debug_data(std::vector<Uint>& row_indices, std::vector<Uint>& col_indices, std::vector<Real>& values) { cf3_assert(m_is_created); }

  //@} END TEST ONLY

private:

  /// state of creation
  bool m_is_created;

  /// number of equations
  Uint m_neq;

  /// number of block rows
  Uint m_blockrow_size;

  /// number of block columns
  Uint m_blockcol_size;

}; // end of class EmptyLSSMatrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_EmptyLSSMatrix_hpp
