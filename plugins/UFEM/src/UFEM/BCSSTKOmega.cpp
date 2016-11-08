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

#include "math/LSS/System.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "BCSSTKOmega.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCWallFunctionSST, common::Action, LibUFEM > BCWallFunctionSST_Builder;

struct ComputeUTau
{
  typedef Real result_type;

  template<typename VelocityT>
  Real operator()(VelocityT& v) const
  {
    typedef typename VelocityT::EtypeT SurfElementT;
    constexpr Uint dim = SurfElementT::dimension;
    typedef Eigen::Matrix<Real, dim, 1>  CoordsT;
    typedef Eigen::Matrix<Real, dim, dim>  JacobianT;

    const mesh::Elements& surface_elements = v.support().elements();
    const Uint surface_idx = v.support().element_idx();
    const auto& face_connectivity = *Handle<mesh::FaceConnectivity const>(surface_elements.get_child_checked("wall_face_connectivity"));
    if(!face_connectivity.has_adjacent_element(surface_idx, 0))
    {
      throw common::SetupError(FromHere(), "No adjacent element for element " + common::to_str(surface_idx) + " on elements " + surface_elements.uri().path());
    }
    const auto adjacent_element = face_connectivity.adjacent_element(surface_idx,0);
    const auto volume_entities = face_connectivity.node_connectivity().entities()[adjacent_element.first];
    const auto& space = volume_entities->geometry_space();
    const CoordsT centroid_mapped_coords = space.shape_function().local_coordinates().colwise().mean();
    std::cout << "centroid mapped coords: " << centroid_mapped_coords.transpose() << std::endl;
    return 1.;
  }
};

static MakeSFOp<ComputeUTau>::type const compute_utau = {};

BCWallFunctionSST::BCWallFunctionSST(const std::string& name) :
  ProtoAction(name),
  rhs(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss")),
  dirichlet(options().option("lss"))
{
  options().add("theta", m_theta)
    .pretty_name("Theta")
    .description("Theta coefficient for the theta-method.")
    .link_to(&m_theta);

  link_physics_constant("kappa", m_kappa);

  FieldVariable<0, ScalarField> k("k", "komega_solution");
  FieldVariable<1, ScalarField> omega("omega", "komega_solution");
  FieldVariable<2, VectorField> u("Velocity", "navier_stokes_solution");

  PhysicsConstant nu("kinematic_viscosity");

  const auto nodal_distance = make_lambda([&](const Uint i, const mesh::Elements* e)
  {

  });

  set_expression(elements_expression
  (
    boost::mpl::vector3<mesh::LagrangeP1::Line2D, mesh::LagrangeP1::Triag3D, mesh::LagrangeP1::Quad3D>(), // Valid for surface element types
    group
    (
      _A(k) = _0, _A(omega) = _0,
      element_quadrature
      (
        _A(omega,omega) += -transpose(N(omega))*N(omega) * compute_utau(u) * _norm(normal)
      ),
      system_matrix +=  m_theta * _A,
      rhs += -_A * _x
    )
  ));
}

BCWallFunctionSST::~BCWallFunctionSST()
{
}

} // namespace UFEM

} // namespace cf3
