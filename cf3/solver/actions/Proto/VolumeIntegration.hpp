// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_VolumeIntegration_hpp
#define cf3_solver_actions_Proto_VolumeIntegration_hpp

#include "SurfaceIntegration.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Compute a volume integral, taking care of the parallel details. Note that result must be zero-initialized
template<typename ResultT, typename ExprT, typename ElementTypesT=mesh::LagrangeP1::CellTypes>
void volume_integral(ResultT& result, mesh::Region& root_region, const ExprT& expr, ElementTypesT etypes=mesh::LagrangeP1::CellTypes())
{
  ResultT local_result = result; // should be 0
  for_each_element<ElementTypesT>(root_region, element_quadrature(boost::proto::lit(local_result) += is_local_element * (expr)));
  detail::reduce_result(result, local_result);

}
template<typename ResultT, typename RegionsT, typename ExprT, typename ElementTypesT=mesh::LagrangeP1::CellTypes>
void volume_integral(ResultT& result, RegionsT&& regions, const ExprT& expr, ElementTypesT etypes=mesh::LagrangeP1::CellTypes())
{
  ResultT zero = result; // should be 0
  for(auto& region : regions)
  {
    ResultT region_result = zero;
    volume_integral(region_result, common::dereference(region), expr, etypes);
    result += region_result;
  }
}

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_VolumeIntegration_hpp
