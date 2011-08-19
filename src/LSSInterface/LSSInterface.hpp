// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef LSSInterface_hpp
#define LSSInterface_hpp

// OBJECTIVE: restrictive and simple to use

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "Common/CommonAPI.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "blockaccumulator.hpp"

#include "LSSMatrix.hpp"
#include "LSSSystem.hpp"
#include "LSSVector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace LSSInterface {

////////////////////////////////////////////////////////////////////////////////////////////

/// @TODO: properly implement component (type_name,ptr,constptr)
class Common_API LSSVector : public Component {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<LSSVector> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<LSSVector const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "LSSVector"; }

  /// Default constructor
  LSSVector(const std::string& name) : Component(name) { }

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint row, const Real value) = 0;

  /// Add value at given location in the matrix
  virtual void add_value(const Uint row, const Real value) = 0;

  /// Get value at given location in the matrix
  virtual void get_value(const Uint row, const Real& value) = 0;

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values to rhs
  virtual void set_rhs_values(const BlockAccumulator& values) = 0;

  /// Add a list of values to rhs
  virtual void add_rhs_values(const BlockAccumulator& values) = 0;

  /// Get a list of values from rhs
  virtual void get_rhs_values(BlockAccumulator& values) = 0;

  /// Set a list of values to sol
  virtual void set_sol_values(const BlockAccumulator& values) = 0;

  /// Add a list of values to sol
  virtual void add_sol_values(const BlockAccumulator& values) = 0;

  /// Get a list of values from sol
  virtual void get_sol_values(BlockAccumulator& values) = 0;

  /// Reset Vector
  virtual void reset(Real reset_to=0.) = 0;

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this vector to screen
  virtual void print_to_screen() = 0;

  /// Print this vector to file
  virtual void print_to_file(const char* fileName) = 0;

  //@} END MISCELLANEOUS

};

////////////////////////////////////////////////////////////////////////////////////////////

/// @TODO: properly implement component (type_name,ptr,constptr)
class Common_API LSSMatrix : public Component {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<LSSMatrix> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<LSSMatrix const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "LSSMatrix"; }

  /// Default constructor
  LSSMatrix(const std::string& name) : Component(name) { }

  /// Setup sparsity structure
  /// should only work with local numbering (parallel computations, plus rcm could be a totally internal matter of the matrix)
  /// internal mapping should be invisible to outside (needs to reorganize to push ghost nodes)
  /// maybe 2 ctable csr style
  /// local numbering
  /// needs global numbering for communication - ??? commpattern ???
  virtual void create_sparsity(Comm::CommPattern& cp, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices) = 0;

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint col, const Uint row, const Real value) = 0;

  /// Add value at given location in the matrix
  virtual void add_value(const Uint col, const Uint row, const Real value) = 0;

  /// Get value at given location in the matrix
  virtual void get_value(const Uint col, const Uint row, Real& value) = 0;

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values) = 0;

  /// Add a list of values
  /// local ibdices
  /// eigen, templatization on top level
  virtual void add_values(const BlockAccumulator& values) = 0;

  /// Add a list of values
  virtual void get_values(BlockAccumulator& values) = 0;

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  virtual void set_row(const Uint blockrow, const Uint blockeqn, Real diagval, Real offdiagval) = 0;

  /// Get a column and replace it to zero (dirichlet-type boundaries, when trying to preserve symmetry)
  /// Note that sparsity info is lost, values will contain zeros where no matrix entry is present
  virtual void get_column_and_replace_to_zero(const Uint col, LSSVector& values) = 0;

  /// Add one line to another and tie to it via dirichlet-style (applying periodicity)
  virtual void tie_row_pairs (const Uint colto, const Uint colfrom) = 0;

  /// Set the diagonal
  virtual void set_diagonal(const LSSVector& diag) = 0;

  /// Add to the diagonal
  virtual void add_diagonal(const LSSVector& diag) = 0;

  /// Get the diagonal
  virtual void get_diagonal(LSSVector& diag) = 0;

  /// Reset Matrix
  virtual void reset(Real reset_to=0.) = 0;

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this matrix
  virtual void print_to_screen() = 0;

  /// Print this matrix to a file
  virtual void print_to_file(const char* fileName) = 0;

  //@} END MISCELLANEOUS

private:

  /// Copy constructor
  LSSMatrix(const LSSMatrix& other);

  /// Overloading of the assignment operator
  const LSSMatrix& operator= (const LSSMatrix& other);

}; // end of class LSSMatrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSSInterface
} // namespace CF

#endif // LSSInterface_hpp
