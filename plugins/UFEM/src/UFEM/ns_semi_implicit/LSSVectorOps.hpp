// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_LSSVectorOps_hpp
#define cf3_UFEM_LSSVectorOps_hpp

#include "solver/ActionDirector.hpp"

#include "../LibUFEM.hpp"
#include "../LSSActionUnsteady.hpp"
#include "../NavierStokesPhysics.hpp"

namespace cf3 {
namespace UFEM {

/// Custom proto op to access element values in an LSS vector for a scalar variable
struct ScalarLSSVector
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename DataT>
  struct result<This(DataT)>
  {
    typedef const Eigen::Matrix<Real, DataT::SupportShapeFunction::nb_nodes, 1>& type;
  };

  template<typename StorageT, typename DataT>
  const StorageT& operator()(StorageT& result, const DataT& data) const
  {
    acc.resize(DataT::SupportShapeFunction::nb_nodes, 1);
    acc.indices = data.block_accumulator.indices;
    vector->get_sol_values(acc);
    result = acc.sol;
    return result;
  }

  // Set the constant to use for setting the matrix
  void set_vector(const Handle<math::LSS::Vector>& v)
  {
    vector = v;
  }

  Handle<math::LSS::Vector> vector;
  mutable math::LSS::BlockAccumulator acc;
};

/// Custom proto op to access element values in an LSS vector for a vector variable
struct VectorLSSVector
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename DataT>
  struct result<This(DataT)>
  {
    typedef const Eigen::Matrix<Real, DataT::dimension, DataT::SupportShapeFunction::nb_nodes>& type;
  };

  template<typename StorageT, typename DataT>
  const StorageT& operator()(StorageT& result, const DataT& data) const
  {
    acc.resize(DataT::SupportShapeFunction::nb_nodes, DataT::dimension);
    acc.indices = data.block_accumulator.indices;
    vector->get_sol_values(acc);
    // We need to renumber to the blocked structure used in the element matrices
    for(Uint i = 0; i != DataT::SupportShapeFunction::nb_nodes; ++i)
    {
      const Uint offset = i*DataT::dimension;
      for(Uint j = 0; j != DataT::dimension; ++j)
      {
        result(j, i) = acc.sol[offset + j];
      }
    }
    return result;
  }

  // Set the constant to use for setting the matrix
  void set_vector(const Handle<math::LSS::Vector>& v)
  {
    vector = v;
  }

  Handle<math::LSS::Vector> vector;
  mutable math::LSS::BlockAccumulator acc;
};
  
} // UFEM
} // cf3


#endif // cf3_UFEM_LSSVectorOps_hpp
