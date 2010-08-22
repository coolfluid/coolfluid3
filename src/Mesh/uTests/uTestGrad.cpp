#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Gradient calculation"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/RealVector.hpp"
#include "Math/MatrixInverterT.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/SF/Quad2DLagrangeP1.hpp"
#include "Mesh/SF/Triag2DLagrangeP1.hpp"


using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Mesh::SF;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct GradTests_Fixture
{
  typedef std::vector<RealVector> NodesT;
  
  /// common setup for each test case
  GradTests_Fixture()  :  qnodes(init_qnodes()), tnodes(init_tnodes())
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
        
  }

  /// common tear-down for each test case
  ~GradTests_Fixture()
  {
  }

  const NodesT qnodes;
  const NodesT tnodes;

  /// Workaround for boost:assign ambiguity
  CF::RealVector init_mapped_coords()
  {
    return list_of(0.1)(0.8);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_qnodes()
  {
    const CF::RealVector c0 = list_of(0.5)(0.3);
    const CF::RealVector c1 = list_of(1.1)(1.2);
    const CF::RealVector c2 = list_of(1.35)(1.9);
    const CF::RealVector c3 = list_of(0.8)(2.1);
    return list_of(c0)(c1)(c2)(c3);
  }
  
  NodesT init_tnodes()
  {
    const CF::RealVector c0 = list_of(0.5)(0.3);
    const CF::RealVector c1 = list_of(1.1)(1.2);
    const CF::RealVector c2 = list_of(0.8)(2.1);
    return list_of(c0)(c1)(c2);
  }
  
  /// create a Real vector with 2 coordinates
  RealVector create_coord(const Real& x, const Real& y) {
    RealVector coordVec(2);
    coordVec[XX]=x;
    coordVec[YY]=y;
    return coordVec;
  }
  
};

/// Calculates the gradient using the given shape function
/// @param nodes The node coordinates for the element
/// @param mapped_coordinates Location where the gradient is to be calculated
/// @param function_values Nodal values of the function for which the gradient will be calculated
template<typename ShapeFunctionT, typename NodesT>
RealVector gradient(const NodesT& nodes, const RealVector& mapped_coordinates, const RealVector& function_values)
{
  // Get the gradient in mapped coordinates
  RealMatrix mapped_grad(ShapeFunctionT::dimensionality,ShapeFunctionT::nb_nodes);
  ShapeFunctionT::mapped_gradient(mapped_coordinates,mapped_grad);
  
  // The Jacobian adjugate
  RealMatrix jacobian_adj(ShapeFunctionT::dimension, ShapeFunctionT::dimensionality);
  ShapeFunctionT::jacobian_adjoint(mapped_coordinates, nodes, jacobian_adj);
  
  // The gradient operator matrix in the absolute frame
  RealMatrix grad(ShapeFunctionT::dimension,ShapeFunctionT::nb_nodes);
  grad = (jacobian_adj * mapped_grad) / ShapeFunctionT::jacobian_determinant(mapped_coordinates, nodes);
  
  // Apply the gradient to the function values
  RealVector result(2);
  result = grad * function_values;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( GradTests_TestSuite, GradTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( GradTestQuad )
{
  CFinfo << "Quad test\n--------------" << CFendl;
  
  // Function values at the node locations are equal to the X coordinate here
  const RealVector function = boost::assign::list_of(qnodes[0][XX])(qnodes[1][XX])(qnodes[2][XX])(qnodes[3][XX]);
  
  // Test at different intervals over the mapped coordinates
  const Uint intervals = 5;
  const Real step = 2. / static_cast<Real>(intervals);
  for(Uint i = 0; i <= intervals; ++i)
  {
    for(Uint j = 0; j <= intervals; ++j)
    {
      RealVector mapped_coords = boost::assign::list_of(-1. + i * step)(-1. + j * step);
      RealVector grad = gradient<Quad2DLagrangeP1>(qnodes, mapped_coords, function);
      CFinfo << "Gradient at " << mapped_coords << " equals " << grad << CFendl;
      BOOST_CHECK_CLOSE(grad[0], 1, 0.00001);
      BOOST_CHECK_SMALL(grad[1], 1e-14);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

// BOOST_AUTO_TEST_CASE( GradTestTriag )
// {
//   CFinfo << "\n\n\nTriangle test\n--------------" << CFendl;
//   
//   CF::RealVector test_coords = list_of(0.8)(1.2);
//   //test_coords = tnodes[0];
//   
//   CF::RealVector mapped_coords(2);
//   Triag::mapped_coordinates(test_coords, tnodes, mapped_coords);
//   CFinfo << "mapped_coords = " << mapped_coords << CFendl;
//   
//   RealMatrix jacob(Triag::dimension,Triag::dimension);  // dx/dksi
//   Triag::jacobian(mapped_coords,tnodes,jacob);
//   CFinfo << "jacob = \n" << jacob << CFendl;
//   
//   RealMatrix inv_jacob(Triag::dimension,Triag::dimension);  // dksi/dx
//   Math::MatrixInverterT<2> inverter;
//   inverter.invert(jacob,inv_jacob);
//   CFinfo << "inv_jacob = \n" << inv_jacob << CFendl;
//   
//   RealMatrix product(Triag::dimension,Triag::dimension);
//   product = inv_jacob*jacob;
//   CFinfo << "inv_jacob*jacob = \n" << product << CFendl;
//   
//   RealMatrix mapped_grad(Triag::nb_nodes,Triag::dimension);
//   Triag::mapped_gradient(mapped_coords,mapped_grad);
//   CFinfo << "mapped_grad = \n" << mapped_grad << CFendl;
//   
//   RealMatrix grad_SF(Triag::nb_nodes,Triag::dimension);
//   grad_SF = mapped_grad*inv_jacob;
//   CFinfo << "grad_SF = \n" << grad_SF << CFendl;
//   
//   Real dXdx = 0;
//   Real dYdy = 0;
//   for (Uint i=0; i<Triag::nb_nodes; ++i)
//   {
//     dXdx += tnodes[i][XX] * grad_SF(i,XX);
//     dYdy += tnodes[i][YY] * grad_SF(i,YY);
//     
//   }
//   CFinfo << "dXdx = " << dXdx << CFendl;
//   CFinfo << "dYdy = " << dYdy << CFendl;
//   
//   BOOST_CHECK_EQUAL(dXdx, 1.0);
//   BOOST_CHECK_EQUAL(dYdy, 1.0);
//   
// }

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

