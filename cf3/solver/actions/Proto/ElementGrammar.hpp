// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ElementGrammar_hpp
#define cf3_solver_actions_Proto_ElementGrammar_hpp

#include <boost/proto/core.hpp>

#include "BlockAccumulator.hpp"
#include "ElementIntegration.hpp"
#include "ElementMatrix.hpp"
#include "ElementTransforms.hpp"
#include "ExpressionGroup.hpp"
#include "IndexLooping.hpp"

/// @file 
/// Grammars related to element-wise mesh operations

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

struct ElementMathBase :
  boost::proto::or_
  <
    NodalValues,
    boost::proto::when // As a special case, fields can be used as a functor taking mapped coordinates. In this case, the interpolated value is returned
    <
      boost::proto::function<FieldTypes, boost::proto::_>,
      InterpolationOp(boost::proto::_value(boost::proto::_child_c<0>), boost::proto::_value(boost::proto::_child_c<1>) )
    >,
    MathTerminals,
    ElementIntegration,
    ElementMatrixGrammar
  >
{
};

struct ElementMath :
  boost::proto::or_
  <
    SFOps< boost::proto::terminal<boost::proto::_> >,
    ElementMathBase,
    ElementMatrixSubBlocks<boost::proto::_>,
    EigenMath<ElementMath, boost::proto::or_<Integers, boost::proto::terminal< IndexTag<boost::proto::_> > > >
  >
{
};

template<typename I, typename J>
struct ElementMathIndexed :
  boost::proto::or_
  <
    IndexValues<I, J>,
    SFOps< boost::proto::call< ElementMathIndexed<I, J> > >,
    ElementMathBase,
    ElementMatrixGrammarIndexed<I, J>,
    EigenMath<boost::proto::call< ElementMathIndexed<I,J> >, boost::proto::or_<Integers, IndexValues<I, J> > >
  >
{
};

template<typename I, typename J>
struct StreamOutputIndexed : StreamOutput< ElementMathIndexed<I, J> >
{
};
  
struct SingleExprElementGrammar :
  boost::proto::or_
  <
    // Assignment to system matrix
    BlockAccumulation<ElementMath>,
    boost::proto::when
    <
      ElementMath,
      IndexLooper<ElementMathIndexed>
    >,
    ElementQuadrature,
    boost::proto::when
    <
      StreamOutput<ElementMath>,
      IndexLooper<StreamOutputIndexed>
    >
  >
{
};

/// Matches and evaluates element-wise expressions
struct ElementGrammar :
  boost::proto::or_
  <
    SingleExprElementGrammar,
    GroupGrammar< SingleExprElementGrammar >
  >
{
};  
} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementGrammar_hpp
