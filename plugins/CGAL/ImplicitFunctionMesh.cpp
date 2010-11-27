// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Implicit_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/Unique_hash_map.h>
#include <CGAL/Handle_hash_function.h>
#include <CGAL/number_utils.h>

#include "Common/Log.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"

#include "CGAL/ImplicitFunctionMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////

CGReal SphereFunction::operator()(const Point& p) const {
  return ::CGAL::squared_distance(p, Point(::CGAL::ORIGIN))-1;
}

////////////////////////////////////////////////////////////////////////////////

/// Converts a CGAL mesh to a coolfluid mesh
template<typename TriangulationComplexT>
void cgal_to_coolfluid(const TriangulationComplexT& complex, CMesh& mesh) {
  // Map CGAL vertex handles to coolfluid point indices
  typedef ::CGAL::Unique_hash_map<typename TriangulationComplexT::Vertex_handle,Uint,::CGAL::Handle_hash_function> VertexMapT;
  VertexMapT vertex_map(0, complex.number_of_cells()); // estimate the number of vertices equal to the cell count


  CRegion& region = mesh.create_region("region");
  
  // coordinate storage
  CTable<Real>& coordinates = *region.create_component_type<CTable<Real> >("coordinates");
  coordinates.initialize(3);
  CTable<Real>::Buffer coordinatesBuffer = coordinates.create_buffer(complex.number_of_cells());
  std::vector<Real> coords_row(3);
  Uint coord_row_count = 0;
  
  // connectivity storage
  CElements& elements = region.create_elements("CF.Mesh.SF.Tetra3DLagrangeP1",coordinates);
  CTable<Uint>::Buffer connBuffer = elements.connectivity_table().create_buffer(complex.number_of_cells());
  std::vector<Uint> cell_row(4);

  CFinfo << "iterating over the cells" << CFendl;

  for(typename TriangulationComplexT::Cell_iterator cell = complex.cells_begin(); cell != complex.cells_end(); ++cell) {
    for(Uint i = 0; i != 4; ++i) {
      typename TriangulationComplexT::Vertex_handle v = cell->vertex(i);
      if(!vertex_map.is_defined(v)) { // store the point, if we didn't have it already
        vertex_map[v] = coord_row_count;
        const Point p = v->point();
        coords_row[XX] = ::CGAL::to_double(p.x());
        coords_row[YY] = ::CGAL::to_double(p.y());
        coords_row[ZZ] = ::CGAL::to_double(p.z());
        coordinatesBuffer.add_row(coords_row);
        ++coord_row_count;
      }
      cell_row[i] = vertex_map[v];
    }
    connBuffer.add_row(cell_row);
  }
  coordinatesBuffer.flush();
  connBuffer.flush();
}

////////////////////////////////////////////////////////////////////////////////

// Implementation is heavily based on the CGAL example
void create_mesh(const ImplicitFunction& function, CMesh& mesh, const MeshParameters parameters) {
  // Domain type
  typedef ::CGAL::Implicit_mesh_domain_3<ImplicitFunction const,KernelT> MeshDomainT;

  // Triangulation
  typedef ::CGAL::Mesh_triangulation_3<MeshDomainT>::type TriangulationT;
  typedef ::CGAL::Mesh_complex_3_in_triangulation_3<TriangulationT> TriangulationComplexT;

  // Criteria
  typedef ::CGAL::Mesh_criteria_3<TriangulationT> MeshCriteriaT;

  // To avoid verbose function and named parameters call
  using namespace ::CGAL::parameters;

  // Domain (Warning: Sphere_3 constructor uses squared radius !)
  Point center(function.x, function.y, function.z);
  MeshDomainT domain(function, KernelT::Sphere_3(center, function.radius*function.radius));

  // Mesh criteria
  MeshCriteriaT criteria(facet_angle=parameters.facet_angle, facet_size=parameters.facet_size, facet_distance=parameters.facet_distance,
                         cell_radius_edge=parameters.cell_radius_edge, cell_size=parameters.cell_size);

  // Mesh generation
  TriangulationComplexT complex = ::CGAL::make_mesh_3<TriangulationComplexT>(domain, criteria);

  CFinfo << "Finished mesh creation" << CFendl;

  // Conversion to coolfluid
  cgal_to_coolfluid(complex, mesh);
}

////////////////////////////////////////////////////////////////////////////////

} // namespace CGAL
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////
