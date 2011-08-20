// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_BlockAccumulator_hpp
#define CF_Math_LSS_BlockAccumulator_hpp

// blockaccumulator would keep local RealMatrix and rhs and solution together
// time splitting could be implemented here?
// multi rhs+sol thinggy?
// blockaccumulator should collect solution beforehand to avoid the mess of looking out for values in the big vector at assembly -> what if for example fvm looks for neighbours?

////////////////////////////////////////////////////////////////////////////////////////////

#include <Math/MatrixTypes.hpp>
#include <Common/CommonAPI.hpp>

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class Common_API BlockAccumulator {
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

  /// entering the indices where the local matrix is lying
  template<typename T> void neighbour_indices(const T& idx_vector )
  {
    CF_ASSERT(indices.size()==idx_vector.size());
    for (Uint i=0; i<(const Uint)indices.size(); i++)
      indices[i]=idx_vector[i];
  };

  /// how many rows/columns
  const Uint size() { return sol.size(); };

  /// accessor to blockaccumulator's RealMatrix
  RealMatrix mat;

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
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_BlockAccumulator_hpp
