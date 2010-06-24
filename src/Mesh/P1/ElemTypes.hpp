#ifndef CF_Mesh_P1_ElemTypes_hpp
#define CF_Mesh_P1_ElemTypes_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/vector.hpp>

#include "Mesh/P1/P1API.hpp"

#include "Mesh/P1/Line1D.hpp"
#include "Mesh/P1/Line2D.hpp"
#include "Mesh/P1/Line3D.hpp"

#include "Mesh/P1/Triag2D.hpp"
#include "Mesh/P1/Triag3D.hpp"

#include "Mesh/P1/Quad2D.hpp"
#include "Mesh/P1/Quad3D.hpp"

#include "Mesh/P1/Hexa3D.hpp"
#include "Mesh/P1/Tetra3D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {

  typedef boost::mpl::vector< Line1D,
                              Line2D,
                              Line3D,
                              Triag2D,
                              Triag3D,
                              Quad2D,
                              Quad3D,
                              Hexa3D,
                              Tetra3D> ElemTypes;

} // P1
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_ElemTypes_hpp
