// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ZeroLSSRow_hpp
#define cf3_solver_actions_Proto_ZeroLSSRow_hpp

#include <boost/proto/core.hpp>

#include "math/MatrixTypes.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Vector.hpp"

#include "BlockAccumulator.hpp"
#include "Transforms.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

struct LSSRowZeroer : boost::proto::callable
{
  typedef void result_type;

  template<typename LSST, typename VarDataT, typename DataT>
  void operator()(LSST& lss, const VarDataT& var_data, const DataT& data)
  {
    const int node_idx = lss.node_to_lss(data.node_idx());
    if(node_idx < 0)
      return;
    const Uint sys_idx = node_idx*var_data.nb_dofs + var_data.offset;
    lss.rhs().set_value(sys_idx, 0.);
    lss.solution().set_value(sys_idx, 0.);
    lss.matrix().set_row(node_idx, var_data.offset, 0., 0.);
  }
};

/// Tag for zero_row function
struct ZeroLSSRowTag
{
};

static boost::proto::terminal<ZeroLSSRowTag>::type zero_row = {};

/// Grammar to evaluate zero_row(system_matrix, variable)
struct ZeroLSSRowGrammar :
  boost::proto::when
  <
    boost::proto::function
    <
      boost::proto::terminal<ZeroLSSRowTag>,
      boost::proto::terminal<LSSWrapperImpl<SystemMatrixTag>>,
      FieldTypes
    >,
    LSSRowZeroer(boost::proto::_value(boost::proto::_child1), VarData(boost::proto::_value(boost::proto::_child2)), boost::proto::_data)
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ZeroLSSRow_hpp
