// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef lss_interface_hpp
#define lss_interface_hpp

// Maybe vector and matrix should be a single class, could be way more performant? But then needs support for multi rhs+solution.
// matrix should access by sparsity given to it, hiding internal renumbering

// OBJECTIVE: restrictive and simple to use

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/utility.hpp>

#include "Common/CommonAPI.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/MPI/PECommPattern.hpp"
#include "blockaccumulator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API LSSVector : boost::noncopyable {
public:

  /// @name CREATION AND DESTRUCTION
  //@{

  /// Default constructor without arguments.
  LSSVector();

  /// Destructor.
  virtual ~LSSVector() = 0;
 
  //@} END CREATION AND DESTRUCTION

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

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values) = 0;

  /// Add a list of values
  virtual void add_values(const BlockAccumulator& values) = 0;

  /// Add a list of values
  virtual void get_values(const BlockAccumulator& values) = 0;

  /// Reset Vector
  virtual void reset_to_zero() = 0;

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

class Common_API LSSMatrix : boost::noncopyable {
public:

  /// @name CREATION AND DESTRUCTION
  //@{

  /// Default constructor without arguments.
  LSSMatrix(){};

  /// Destructor.
  ~LSSMatrix(){};

  /// Setup sparsity structure
  /// should only work with local numbering (parallel computations, plus rcm could be a totally internal matter of the matrix)
  /// internal mapping should be invisible to outside (needs to reorganize to push ghost nodes)
  /// maybe 2 ctable csr style
  /// local numbering
  /// needs global numbering for communication - ??? commpattern ???
  virtual void create_sparsity(PECommPattern& cp, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices) = 0;

  //@} END CREATION AND DESTRUCTION

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
  virtual void set_row(const Uint row, Real diagval, Real offdiagval) = 0;

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
  virtual void reset_to_zero() = 0;

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

} // namespace Common
} // namespace CF

#endif // lss_interface_hpp
