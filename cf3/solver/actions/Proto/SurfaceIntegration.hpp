// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_SurfaceIntegration_hpp
#define cf3_solver_actions_Proto_SurfaceIntegration_hpp

#include "common/DereferenceComponent.hpp"

#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "ElementLooper.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Compute a surface integral, taking care of the parallel details. Note that result must be zero-initialized
template<typename ResultT, typename ExprT, typename ElementTypesT=mesh::LagrangeP1::FaceTypes>
void surface_integral(ResultT& result, mesh::Region& root_region, const ExprT& expr, ElementTypesT etypes=mesh::LagrangeP1::FaceTypes())
{
  ResultT local_result = result; // should be 0
  for_each_element<ElementTypesT>(root_region, element_quadrature(boost::proto::lit(local_result) += is_local_element * (expr)));

  if(common::PE::Comm::instance().is_active())
  {
    common::PE::Comm::instance().all_reduce(common::PE::plus(), &local_result, 1, &result);
  }
  else
  {
    result = local_result;
  }
}
template<typename ResultT, typename RegionsT, typename ExprT, typename ElementTypesT=mesh::LagrangeP1::FaceTypes>
void surface_integral(ResultT& result, RegionsT&& regions, const ExprT& expr, ElementTypesT etypes=mesh::LagrangeP1::FaceTypes())
{
  ResultT zero = result; // should be 0
  for(auto& region : regions)
  {
    ResultT region_result = zero;
    surface_integral(region_result, common::dereference(region), expr, etypes);
    result += region_result;
  }
}

/// Compute the area of a mesh region, using a Proto expression and taking care of the parallel details
template<typename ElementTypesT=mesh::LagrangeP1::FaceTypes>
Real compute_area(mesh::Region& root_region, ElementTypesT etypes=mesh::LagrangeP1::FaceTypes())
{
  Real result = 0;
  surface_integral(result, root_region, _norm(normal), etypes);
  return result;
}
template<typename RegionsT, typename ElementTypesT=mesh::LagrangeP1::FaceTypes>
Real compute_area(RegionsT&& regions, ElementTypesT etypes=mesh::LagrangeP1::FaceTypes())
{
  Real result = 0;
  surface_integral(result, std::forward<RegionsT>(regions), _norm(normal), etypes);
  return result;
}

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_SurfaceIntegration_hpp
