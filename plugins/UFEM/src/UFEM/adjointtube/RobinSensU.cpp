// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>
#include <vector>
#include <iostream>
#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "RobinSensU.hpp"
#include "../AdjacentCellToFace.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"


namespace cf3
{

namespace UFEM
{
namespace adjointtube
{
using namespace solver::actions::Proto;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RobinSensU, common::Action, LibUFEMAdjointTube > RobinSensU_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

using boost::proto::lit;

namespace detail
{

struct ComputeDivn
{
  typedef Real result_type;

  template<typename NodalNormalT>
  Real operator()(const NodalNormalT& n) const
  {
    const RealVector1 mapped_coords(0.0); // xi
    typedef typename NodalNormalT::EtypeT EtypeT;
    typename EtypeT::SF::GradientT mapped_gradient_matrix;
    std::cout << "n:" << n.value() << std::endl;
    auto normalized_n = n.value();
    normalized_n.row(0) /= normalized_n.row(0).norm();
    normalized_n.row(1) /= normalized_n.row(1).norm();
    std::cout << "normalized n:" << normalized_n << std::endl;
    std::cout << "mapped gradient matrix:" << mapped_gradient_matrix << std::endl;
    EtypeT::SF::compute_gradient(mapped_coords, mapped_gradient_matrix);
    //const auto d_n_d_xi = mapped_gradient_matrix*normalized_n; // [dnx/dxi dny/dxi]
    const auto d_n_d_xi_0 = mapped_gradient_matrix[0]; // [dnx/dxi dny/dxi]
    const auto d_n_d_xi_1 = mapped_gradient_matrix[1];
    std::cout << "Dn / Dxi:" << d_n_d_xi_0 << std::endl;
    const auto jacobian = n.support().jacobian(mapped_coords);
    std::cout << "jacobiaan:" << jacobian << std::endl;
    const auto Res1 = d_n_d_xi_0 / jacobian[0];
    std::cout << "1ste term:" << Res1 << std::endl;
    return  d_n_d_xi_1 / jacobian[1];

  }
};

static MakeSFOp<ComputeDivn>::type const divn = {};

}

RobinSensU::RobinSensU(const std::string& name) :
  Action(name),
  system_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))


{

   const auto dimension = make_lambda([](const RealVector& u)
   {
    if(u.size() == 3)
    {
     return 1;
     }
      return 0;

    });

    //Handle<AdjacentCellToFace> set_boundary_gradient(get_child("SetBoundaryGradient"));

    auto robincondition = create_static_component<ProtoAction>("Robincondition");

    // Represents the temperature field, as calculated
    FieldVariable<0, VectorField> SensU("SensU","sensitivity_solution");
    FieldVariable<1, ScalarField> SensP("SensP","sensitivity_solution");
    FieldVariable<2, VectorField> n("NodalNormal", "nodal_normals");

        robincondition->set_expression(elements_expression
        (
         boost::mpl::vector<mesh::LagrangeP1::Line2D

                             >(), // Valid for surface element types
         group
         (
         _A(SensU) = _0, _A(SensP) = _0,
         //compute_tau.apply(lit(tau_su))
         _A(SensU[_i],SensU[_i]) = integral<2>(transpose(N(SensU))*N(SensU)*detail::divn(n)*_norm(normal)),//
         //
                system_matrix+=_A,
                system_rhs += -_A*_x
        )));



}

RobinSensU::~RobinSensU()
{
}


void RobinSensU::on_regions_set()
{
  get_child("Robincondition")->options().set("regions", options()["regions"].value());

}




void RobinSensU::execute(){

    Handle<ProtoAction> robincondition(get_child("Robincondition"));
    robincondition->execute();

}
} // namespace adjointtube
} // namespace UFEM

} // namespace cf3
