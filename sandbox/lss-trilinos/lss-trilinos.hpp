// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef lss_trilinos_hpp
#define lss_trilinos_hpp

#include <Epetra_MpiComm.h>

#include "lss-interface.hpp"


////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API LSSTrilinosVector : public LSSVector {
public:

  /// @name CREATION AND DESTRUCTION
  //@{

  /// Default constructor without arguments.
  LSSTrilinosVector();

  /// Destructor.
  virtual ~LSSTrilinosVector();

  //@} END CREATION AND DESTRUCTION

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint row, const Real value){};

  /// Add value at given location in the matrix
  virtual void add_value(const Uint row, const Real value){};

  /// Get value at given location in the matrix
  virtual void get_value(const Uint row, const Real& value){};

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void add_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void get_values(const BlockAccumulator& values){};

  /// Reset Vector
  virtual void reset_to_zero(){};

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this vector to screen
  virtual void print_to_screen(){};

  /// Print this vector to file
  virtual void print_to_file(const char* fileName){};

  //@} END MISCELLANEOUS

};

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API LSSTrilinosMatrix : public LSSMatrix {
public:

  /// @name CREATION AND DESTRUCTION
  //@{

  /// Default constructor without arguments.
  LSSTrilinosMatrix():
    m_comm(mpi::PE::instance().communicator())
  {
  };

  /// Destructor.
  ~LSSTrilinosMatrix(){};

  /// Setup sparsity structure
  /// should only work with local numbering (parallel computations, plus rcm could be a totally internal matter of the matrix)
  /// internal mapping should be invisible to outside (needs to reorganize to push ghost nodes)
  /// maybe 2 ctable csr style
  /// local numbering
  /// needs global numbering for communication - ??? commpattern ???
  virtual void create_sparsity(PECommPattern& cp, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
  {
    /// @todo don't hardcode the name, rather write an accessor to it in pecommpattern
    Uint *gid=(Uint*)cp.get_child_ptr("gid")->as_ptr<PEObjectWrapper>()->pack();


  }

  //@} END CREATION AND DESTRUCTION

  /// @name INDIVIDUAL ACCESS
  //@{

  /// Set value at given location in the matrix
  virtual void set_value(const Uint col, const Uint row, const Real value){};

  /// Add value at given location in the matrix
  virtual void add_value(const Uint col, const Uint row, const Real value){};

  /// Get value at given location in the matrix
  virtual void get_value(const Uint col, const Uint row, Real& value){};

  //@} END INDIVIDUAL ACCESS

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  virtual void set_values(const BlockAccumulator& values){};

  /// Add a list of values
  /// local ibdices
  /// eigen, templatization on top level
  virtual void add_values(const BlockAccumulator& values){};

  /// Add a list of values
  virtual void get_values(BlockAccumulator& values){};

  /// Set a row, diagonal and off-diagonals values separately (dirichlet-type boundaries)
  virtual void set_row(const Uint row, Real diagval, Real offdiagval){};

  /// Get a column and replace it to zero (dirichlet-type boundaries, when trying to preserve symmetry)
  /// Note that sparsity info is lost, values will contain zeros where no matrix entry is present
  virtual void get_column_and_replace_to_zero(const Uint col, LSSVector& values){};

  /// Add one line to another and tie to it via dirichlet-style (applying periodicity)
  virtual void tie_row_pairs (const Uint colto, const Uint colfrom){};

  /// Set the diagonal
  virtual void set_diagonal(const LSSVector& diag){};

  /// Add to the diagonal
  virtual void add_diagonal(const LSSVector& diag){};

  /// Get the diagonal
  virtual void get_diagonal(LSSVector& diag){};

  /// Reset Matrix
  virtual void reset_to_zero(){};

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print this matrix
  virtual void print_to_screen(){};

  /// Print this matrix to a file
  virtual void print_to_file(const char* fileName){};

  //@} END MISCELLANEOUS

private:

  Epetra_MpiComm m_comm;

}; // end of class LSSTrilinosMatrix

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

#endif // lss_trilinos_hpp
