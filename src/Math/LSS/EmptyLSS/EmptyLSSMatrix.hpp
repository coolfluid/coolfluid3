// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_EmptyLSSMatrix_hpp
#define CF_Math_LSS_EmptyLSSMatrix_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "Math/LibMath.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/Matrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file EmptyLSSMatrix.hpp implementation of LSS::EmptyLSSMatrix
  @author Tamas Banyai

  EmptyLSSMatrix is intended to use for testing purposes only.
  It acts like a fully operational linear solver but it does not solve and allocate any memory.
  // @todo turn it into a testing suite and throws for everything incorrect
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class Math_API EmptyLSSMatrix : public LSS::Matrix {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<EmptyLSSMatrix> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<EmptyLSSMatrix const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "EmptyLSSMatrix"; }

  /// Default constructor
  EmptyLSSMatrix(const std::string& name) :
    LSS::Matrix(name),
    m_blockcol_size(0),
    m_blockrow_size(0),
    m_neq(0),
    m_is_created(false),
    m_solvertype("EmptyLSS")
  { }

  /// Setup sparsity structure
  void create(CF::Common::Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
  {
    destroy();
    m_neq=neq;
    m_blockrow_size=cp.gid()->size();
    m_blockcol_size=(*max_element(node_connectivity.begin(),node_connectivity.end(),std::less<Uint>()))+1;
    m_is_created=true;
  }

  /// Deallocate underlying data
  void destroy() { m_is_created=false; }

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  void set_value(const Uint icol, const Uint irow, const Real value) { cf_assert(m_is_created); }

  /// Add value at given location in the matrix
  void add_value(const Uint icol, const Uint irow, const Real value) { cf_assert(m_is_created); }

  /// Get value at given location in the matrix
  void get_value(const Uint icol, const Uint irow, Real& value) {  cf_assert(m_is_created); value=0.; }

  //@} END INDIVIDUAL ACCESS

  /// @name SOLVE THE SYSTEM
  //@{

  /// The holy solve, for solving the m_mat*m_sol=m_rhs problem.
  /// We bow on our knees before your greatness.
  void solve(LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs) { cf_assert(m_is_created); }

  //@} END SOLVE THE SYSTEM

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  void set_values(const BlockAccumulator& values) { cf_assert(m_is_created); }

  /// Add a list of values
  /// local ibdices
  /// eigen, templatization on top level
  void add_values(const BlockAccumulator& values) { cf_assert(m_is_created); }

  /// Add a list of values
  void get_values(BlockAccumulator& values) { cf_assert(m_is_created); values.mat.setConstant(0.); }

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  void set_row(const Uint iblockrow, const Uint ieq, Real diagval, Real offdiagval) { cf_assert(m_is_created); }

  /// Get a column and replace it to zero (dirichlet-type boundaries, when trying to preserve symmetry)
  /// Note that sparsity info is lost, values will contain zeros where no matrix entry is present
  void get_column_and_replace_to_zero(const Uint iblockcol, Uint ieq, std::vector<Real>& values) { cf_assert(m_is_created); values.resize(m_blockcol_size*m_neq,0.); }

  /// Add one line to another and tie to it via dirichlet-style (applying periodicity)
  void tie_blockrow_pairs (const Uint iblockrow_to, const Uint iblockrow_from) { cf_assert(m_is_created); }

  /// Set the diagonal
  void set_diagonal(const std::vector<Real>& diag) { cf_assert(m_is_created); cf_assert(diag.size()==m_blockrow_size*m_neq); }

  /// Add to the diagonal
  void add_diagonal(const std::vector<Real>& diag) { cf_assert(m_is_created); cf_assert(diag.size()==m_blockrow_size*m_neq); }

  /// Get the diagonal
  void get_diagonal(std::vector<Real>& diag) {  cf_assert(m_is_created); diag.assign(m_blockrow_size*m_neq,0.); }

  /// Reset Matrix
  void reset(Real reset_to=0.) { cf_assert(m_is_created); }

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  void print(Common::LogStream& stream) { cf_assert(m_is_created); stream << "EmptyLSSMatrix::print of '" << name() << "'.\n"; }

  /// Print to wherever
  void print(std::ostream& stream) { cf_assert(m_is_created); stream << "EmptyLSSMatrix::print of '" << name() << "'.\n"; }

  /// Print to file given by filename
  void print(const std::string& filename) { cf_assert(m_is_created); }

  /// Accessor to the state of create
  const bool is_created() { return m_is_created; }

  /// Accessor to the number of equations
  const Uint neq() {  cf_assert(m_is_created); return m_neq; }

  /// Accessor to the number of block rows
  const Uint blockrow_size() {  cf_assert(m_is_created); return m_blockrow_size; }

  /// Accessor to the number of block columns
  const Uint blockcol_size() {  cf_assert(m_is_created); return m_blockcol_size; }

  /// Accessor to solver type
  virtual const std::string solvertype() { return m_solvertype; }

  //@} END MISCELLANEOUS

private:

  /// state of creation
  bool m_is_created;

  /// number of equations
  Uint m_neq;

  /// number of block rows
  Uint m_blockrow_size;

  /// number of block columns
  Uint m_blockcol_size;

  /// type of solver
  std::string m_solvertype;

}; // end of class EmptyLSSMatrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_EmptyLSSMatrix_hpp
