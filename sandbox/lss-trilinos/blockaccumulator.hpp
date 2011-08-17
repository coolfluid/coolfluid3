// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef blockaccumulator_hpp
#define blockaccumulator_hpp

// blockaccumulator would keep local RealMatrix and rhs and solution together
// time splitting could be implemented here?
// multi rhs+sol thinggy?
// blockaccumulator should collect solution beforehand to avoid the mess of looking out for values in the big vector at assembly -> what if for example fvm looks for neighbours?

////////////////////////////////////////////////////////////////////////////////////////////

#include <Math/MatrixTypes.hpp>
#include <Common/CommonAPI.hpp>

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API BlockAccumulator {
public:

  /// setting up sizes
  void prepare(Uint numnodes, Uint numeqs)
  {
    const Uint size=numnodes*numeqs;
    m_mat.resize(size,size);
    m_sol.resize(size);
    m_rhs.resize(size);
    m_row_idxs.resize(numnodes);
    m_col_idxs.resize(numnodes);
  };

  /// how many rows
  const Uint num_rows() { return m_mat.size(); };

  /// how many columns
  const Uint num_cols() { return m_mat.size(); };

  /// local numbering of the rows
  RealVector& row_idxs() { return m_row_idxs; };

  /// local numbering of the columns
  RealVector& col_idxs() { return m_col_idxs; };

  /// accessor to blockaccumulator's RealMatrix
  RealMatrix& mat() { return m_mat; };

  /// accessor to blockaccumulator's solution vector
  RealVector& sol() { return m_sol; };

  /// accessor to blockaccumulator's right hand side vector
  RealVector& rhs() { return m_rhs; };

  // rest of the operations should directly be the stuff off eigen

private:

  /// data holders are directly from eigen
  RealMatrix m_mat;
  RealVector m_sol;
  RealVector m_rhs;
  RealVector m_row_idxs;
  RealVector m_col_idxs;

  /// private constructor

};

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

#endif // blockaccumulator_hpp
