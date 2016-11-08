// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_NodalMatrixManipulation_hpp
#define cf3_solver_actions_Proto_NodalMatrixManipulation_hpp

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

struct MatrixSetter : boost::proto::callable
{
  typedef void result_type;

  template<typename LSST, typename RowVarDataT, typename ColVarDataT, typename DataT, typename ValT>
  void operator()(LSST& lss, const RowVarDataT& row_var_data, const ColVarDataT& col_var_data, const DataT& data, const ValT& value)
  {
    static_assert(ColVarDataT::dimension == ValT::ColsAtCompileTime, "Invalid number of columns in nodal matrix assignment");
    static_assert(RowVarDataT::dimension == ValT::RowsAtCompileTime, "Invalid number of rows in nodal matrix assignment");

    const int node_idx = lss.node_to_lss(data.node_idx());
    if(node_idx < 0)
      return;

    const Uint rows_begin = node_idx*row_var_data.nb_dofs + row_var_data.offset;
    const Uint cols_begin = node_idx*col_var_data.nb_dofs + col_var_data.offset;

    for(Uint i = 0; i != RowVarDataT::dimension; ++i)
    {
      for(Uint j = 0; j != ColVarDataT::dimension; ++j)
      {
        lss.matrix().set_value(cols_begin+j, rows_begin+i, value(i,j));
      }
    }
  }

  template<typename LSST, typename RowVarDataT, typename ColVarDataT, typename DataT>
  void operator()(LSST& lss, const RowVarDataT& row_var_data, const ColVarDataT& col_var_data, const DataT& data, const Real value)
  {
    const int node_idx = lss.node_to_lss(data.node_idx());
    if(node_idx < 0)
      return;

    static_assert(RowVarDataT::dimension == 1, "Real value must be assigned to scalar variables");
    static_assert(ColVarDataT::dimension == 1, "Real value must be assigned to scalar variables");
  }
};

/// Grammar to evaluate zero_row(system_matrix, variable)
template<typename GrammarT>
struct NodalMatrixGrammar :
  boost::proto::when
  <
    boost::proto::assign
    <
      boost::proto::function
      <
        boost::proto::terminal<LSSWrapperImpl<SystemMatrixTag>>,
        FieldTypes,
        FieldTypes
      >,
      GrammarT
    >,
    MatrixSetter
    (
      boost::proto::_value(boost::proto::_child0(boost::proto::_left)),
      VarData(boost::proto::_value(boost::proto::_child1(boost::proto::_left))),
      VarData(boost::proto::_value(boost::proto::_child2(boost::proto::_left))),
      boost::proto::_data,
      GrammarT(boost::proto::_right)
    )
  >
{
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_NodalMatrixManipulation_hpp
