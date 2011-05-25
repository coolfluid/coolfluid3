// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef blockaccumulator_hpp
#define blockaccumulator_hpp

// blockaccumulator would keep local matrix and rhs and solution together
// time splitting could be implemented here?
// multi rhs+sol thinggy?
// blockaccumulator should collect solution beforehand to avoid the mess of looking out for values in the big vector at assembly -> what if for example fvm looks for neighbours?

template <typename T> class blockaccumulator {
public:

  /// how many rows
  const Uint num_rows();

  /// how many columns
  const Uint num_cols();

  /// local numbering of the rows
  const Uint* row_idxs();

  /// local numbering of the columns
  const Uint* col_idxs();

  /// accessor to blockaccumulator's matrix
  /// maybe made friends with lssmatrix
  Matrix<T,Dynamic,Dynamic> get_blockaccumulator_matrix();

  /// accessor to blockaccumulator's solution vector
  /// maybe made friends with lssvector
  Matrix<T,Dynamic,1> get_blockaccumulator_sol();

  /// accessor to blockaccumulator's right hand side vector
  /// maybe made friends with lssvector
  Matrix<T,Dynamic,1> get_blockaccumulator_rhs();

  // rest of the operations should directly be the stuff off eigen

private:

  /// data holders are directly from eigen
  Matrix<T,Dynamic,Dynamic> mat;
  Matrix<T,Dynamic,1> sol;
  Matrix<T,Dynamic,1> rhs;

};

#endif // blockaccumulator_hpp
