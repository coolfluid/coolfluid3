// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/Builder.hpp"
#include "EmptyStrategy.hpp"

namespace cf3 {
namespace math {
namespace LSS {

common::ComponentBuilder<EmptyStrategy, SolutionStrategy, LibLSS> EmptyStrategy_builder;

EmptyStrategy::EmptyStrategy(const std::string& name): SolutionStrategy(name)
{
}

EmptyStrategy::~EmptyStrategy()
{
}

Real EmptyStrategy::compute_residual()
{
  return 0.;
}

void EmptyStrategy::set_matrix(const Handle< Matrix >& matrix)
{
}

void EmptyStrategy::set_rhs(const Handle< Vector >& rhs)
{
}

void EmptyStrategy::set_solution(const Handle< Vector >& solution)
{
}

void EmptyStrategy::solve()
{
}

} // namespace LSS
} // namespace math
} // namespace cf3

