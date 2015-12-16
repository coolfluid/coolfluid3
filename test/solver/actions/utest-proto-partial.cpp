// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto partial derivatives in index notation"

#include <boost/test/unit_test.hpp>

#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "solver/actions/Proto/ElementLooper.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "common/Core.hpp"
#include "common/Log.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/Integrators/Gauss.hpp"
#include "mesh/ElementTypes.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::solver::actions::Proto;

////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( PartialCalls )

using boost::proto::lit;

BOOST_AUTO_TEST_CASE(IndexedDerivative)
{
  Handle<mesh::Mesh> mesh = common::Core::instance().root().create_component<mesh::Mesh>("QuadGridIndexedDerivative");
  Tools::MeshGeneration::create_rectangle(*mesh, 1., 1., 1, 1);

  mesh->geometry_fields().create_field( "vector", "u[vector]" ).add_tag("vector");
  mesh->geometry_fields().create_field( "scalar", "phi" ).add_tag("scalar");

  FieldVariable<0, VectorField > u("u", "vector");
  FieldVariable<1, ScalarField > phi("phi", "scalar");

  for_each_node(mesh->topology(), group(u[0] = 2.*coordinates[0],
                                        u[1] = 4.*coordinates[1] + coordinates[0],
                                        phi = 2.*coordinates[0] + coordinates[1]));

  Real div_result1 = 0.;
  Real div_result2 = 0.;
  RealVector2 centroid; centroid.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(div_result1) += partial(u[_i], _i)),
      lit(div_result1) = div_result1 / volume,
      lit(div_result2) = partial(u[_i], _i, centroid)
    )
  );

  BOOST_CHECK_EQUAL(div_result1, 6.);
  BOOST_CHECK_EQUAL(div_result2, 6.);

  RealMatrix2 grad_vec_result1, grad_vec_result2;
  grad_vec_result1.setZero(); grad_vec_result2.setZero();

  const RealMatrix2 grad_ref = (RealMatrix2() <<
   2., 1.,
   0., 4.).finished();

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(grad_vec_result1) += partial(u[_i], _j)),
      lit(grad_vec_result1) = grad_vec_result1 / volume,
      lit(grad_vec_result2) = partial(u[_i], _j, centroid)
    )
  );

  BOOST_CHECK_EQUAL(grad_vec_result1, grad_vec_result2);
  BOOST_CHECK_EQUAL(grad_vec_result1, grad_ref);

  grad_vec_result1.setZero();
  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(grad_vec_result1) += partial(u[_j], _i)),
      lit(grad_vec_result1) = grad_vec_result1 / volume,
      lit(grad_vec_result2) = partial(u[_j], _i, centroid)
    )
  );

  BOOST_CHECK_EQUAL(grad_vec_result1, grad_vec_result2);
  BOOST_CHECK_EQUAL(grad_vec_result1, grad_ref.transpose());

  RealVector2 grad_scalar_result1, grad_scalar_result2;
  grad_scalar_result1.setZero(); grad_scalar_result2.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(grad_scalar_result1) += partial(phi, _i)),
      lit(grad_scalar_result1) = grad_scalar_result1 / volume,
      lit(grad_scalar_result2) = partial(phi, _i, centroid)
    )
  );

  BOOST_CHECK_CLOSE(grad_scalar_result1[0], grad_scalar_result2[0], 1e-6);
  BOOST_CHECK_CLOSE(grad_scalar_result1[1], grad_scalar_result2[1], 1e-6);
  BOOST_CHECK_CLOSE(grad_scalar_result1[0], 2, 1e-6);
  BOOST_CHECK_CLOSE(grad_scalar_result1[1], 1, 1e-6);

  RealMatrix grad_scalar_tp_result1(1,2), grad_scalar_tp_result2(1,2);
  grad_scalar_tp_result1.setZero(); grad_scalar_tp_result2.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(grad_scalar_tp_result1) += partial(phi, _j)),
      lit(grad_scalar_tp_result1) = grad_scalar_tp_result1 / volume,
      lit(grad_scalar_tp_result2) = partial(phi, _j, centroid)
    )
  );

  BOOST_CHECK_CLOSE(grad_scalar_tp_result1(0,0), grad_scalar_tp_result2(0,0), 1e-6);
  BOOST_CHECK_CLOSE(grad_scalar_tp_result1(0,1), grad_scalar_tp_result2(0,1), 1e-6);
  BOOST_CHECK_CLOSE(grad_scalar_tp_result1(0,0), 2, 1e-6);
  BOOST_CHECK_CLOSE(grad_scalar_tp_result1(0,1), 1, 1e-6);

  BOOST_CHECK_EQUAL(count_repeating_index<0>(partial(u[_i], _j)).value, 1);
  BOOST_CHECK_EQUAL(count_repeating_index<1>(partial(u[_i], _j)).value, 1);
  BOOST_CHECK_EQUAL(count_repeating_index<0>(partial(u[_i], _i)).value, 2);
  BOOST_CHECK_EQUAL(count_repeating_index<1>(partial(u[_i], _i)).value, 0);

  BOOST_CHECK_EQUAL(count_repeating_index<0>(partial(u[_j], _i)).value, 1);
  BOOST_CHECK_EQUAL(count_repeating_index<1>(partial(u[_j], _i)).value, 1);
  BOOST_CHECK_EQUAL(count_repeating_index<0>(partial(u[_j], _j)).value, 0);
  BOOST_CHECK_EQUAL(count_repeating_index<1>(partial(u[_j], _j)).value, 2);

  BOOST_CHECK_EQUAL(count_repeating_index<0>(partial(phi, _i)).value, 1);
  BOOST_CHECK_EQUAL(count_repeating_index<1>(partial(phi, _i)).value, 0);
  BOOST_CHECK_EQUAL(count_repeating_index<0>(partial(phi, _j)).value, 0);
  BOOST_CHECK_EQUAL(count_repeating_index<1>(partial(phi, _j)).value, 1);
}

BOOST_AUTO_TEST_CASE(PartialProducts)
{
  Handle<mesh::Mesh> mesh = common::Core::instance().root().create_component<mesh::Mesh>("QuadGridPartialProducts");
  Tools::MeshGeneration::create_rectangle(*mesh, 1., 1., 1, 1);

  mesh->geometry_fields().create_field( "vector", "u[vector]" ).add_tag("vector");
  mesh->geometry_fields().create_field( "scalar", "phi" ).add_tag("scalar");

  FieldVariable<0, VectorField > u("u", "vector");
  FieldVariable<1, ScalarField > phi("phi", "scalar");

  for_each_node(mesh->topology(), group(u[0] = 2.*coordinates[0],
                                        u[1] = 4.*coordinates[1] + coordinates[0],
                                        phi = 2.*coordinates[0] + coordinates[1]));

  Real scalar_result = 0.;
  RealVector2 centroid; centroid.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(scalar_result) += partial(u[_i], _i) * partial(u[_j], _j)),
      lit(scalar_result) = scalar_result / volume
    )
  );

  BOOST_CHECK_EQUAL(scalar_result, 36.);
  scalar_result = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(scalar_result) += (partial(u[_i], _j) + partial(u[_j], _i)) * partial(u[_i], _j)),
      lit(scalar_result) = scalar_result / volume
    )
  );

  BOOST_CHECK_EQUAL(scalar_result, 41.);
  scalar_result = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(scalar_result) += partial(phi, _i) * partial(phi, _i)),
      lit(scalar_result) = scalar_result / volume
    )
  );

  BOOST_CHECK_CLOSE(scalar_result, 5., 1e-6);
  scalar_result = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      element_quadrature(lit(scalar_result) += partial(phi, _j) * partial(phi, _j)),
      lit(scalar_result) = scalar_result / volume
    )
  );

  BOOST_CHECK_CLOSE(scalar_result, 5., 1e-6);
  scalar_result = 0;
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
