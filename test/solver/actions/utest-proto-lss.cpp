// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "solver/Model.hpp"
#include "solver/Solver.hpp"

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

#include "physics/PhysModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::mesh;
using namespace cf3::common;

using namespace cf3::math::Consts;

/// Check close, for testing purposes
inline void check_close(const RealMatrix2& a, const RealMatrix2& b, const Real threshold)
{
  for(Uint i = 0; i != a.rows(); ++i)
    for(Uint j = 0; j != a.cols(); ++j)
      BOOST_CHECK_CLOSE(a(i,j), b(i,j), threshold);
}

static boost::proto::terminal< void(*)(const RealMatrix2&, const RealMatrix2&, Real) >::type const _check_close = {&check_close};

////////////////////////////////////////////////////

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector3< LagrangeP1::Line1D,
                             LagrangeP1::Quad2D,
                             LagrangeP1::Hexa3D
> HigherIntegrationElements;

typedef boost::mpl::vector5< LagrangeP1::Line1D,
                             LagrangeP1::Triag2D,
                             LagrangeP1::Quad2D,
                             LagrangeP1::Hexa3D,
                             LagrangeP1::Tetra3D
> VolumeTypes;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

using boost::proto::lit;

//////////////////////////////////////////////////////////////////////////////

struct CustomTerminal
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename DataT>
  struct result<This(DataT)>
  {
    typedef const Eigen::Matrix<Real, DataT::SupportShapeFunction::nb_nodes, DataT::SupportShapeFunction::nb_nodes>& type;
  };

  template<typename StorageT, typename DataT>
  const StorageT& operator()(StorageT& result, const DataT& field) const
  {
    result.setZero();
    return result;
  }
};

MakeSFOp<CustomTerminal>::type my_term = {};

BOOST_AUTO_TEST_CASE( MyTerminal )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    _cout << my_term << "\n"
  );

}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
