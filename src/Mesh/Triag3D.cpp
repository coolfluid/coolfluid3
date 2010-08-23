#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Triag3D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

Triag3D::Triag3D()
{
  m_shape = shape;
  m_dimension = dimension;
  m_dimensionality = dimensionality;
  m_nb_faces = nb_faces;
  m_nb_edges = nb_edges;
}

////////////////////////////////////////////////////////////////////////////////

// Define the members so functions taking a reference to these work.
// See http://stackoverflow.com/questions/272900/c-undefined-reference-to-static-class-member
const GeoShape::Type Triag3D::shape;
const Uint Triag3D::nb_faces;
const Uint Triag3D::nb_edges;
const Uint Triag3D::dimensionality;
const Uint Triag3D::dimension;

} // Mesh
} // CF
