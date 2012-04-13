// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_BlockAccumulator_hpp
#define cf3_Math_LSS_BlockAccumulator_hpp

// blockaccumulator would keep local RealMatrix and rhs and solution together
// time splitting could be implemented here?
// multi rhs+sol thinggy?
// blockaccumulator should collect solution beforehand to avoid the mess of looking out for values in the big vector at assembly -> what if for example fvm looks for neighbours?

////////////////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "math/LSS/LibLSS.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class LSS_API BlockAccumulator {
public:

  /// setting up sizes
  void resize(Uint numnodes, Uint numeqs)
  {
    const Uint size=numnodes*numeqs;
    mat.resize(size,size);
    sol.resize(size);
    rhs.resize(size);
    indices.resize(numnodes);
  };

  /// reset the values to the value of reset_to
  void reset(Real reset_to=0.)
  {
    mat.setConstant(reset_to);
    sol.setConstant(reset_to);
    rhs.setConstant(reset_to);
  }

  /// entering the indices where the local matrix is lying
  template<typename T> void neighbour_indices(const T& idx_vector )
  {
    cf3_assert(indices.size()==idx_vector.size());
    for (Uint i=0; i<(const Uint)indices.size(); i++)
      indices[i]=idx_vector[i];
  };

  /// how many rows/columns
  Uint size() const { return sol.size(); };

  /// how many rows/columns
  Uint block_size() const { return indices.size(); };

  /// accessor to blockaccumulator's RealMatrix
  /// Note: TrilinosCrsMatrix assumes row-major storage order here!
  Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> mat;

  /// accessor to blockaccumulator's solution vector
  RealVector sol;

  /// accessor to blockaccumulator's right hand side vector
  RealVector rhs;

  /// local numbering of the unknowns
  std::vector<Uint> indices;

  // rest of the operations should directly be the stuff off eigen

};

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_BlockAccumulator_hpp
