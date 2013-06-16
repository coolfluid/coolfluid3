// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "common/Log.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Cells.hpp"

#include "CGAL/ImplicitFunctionMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////

CGReal SphereFunction::operator()(const Point& p) const {
  return ::CGAL::squared_distance(p, Point(::CGAL::ORIGIN))-1;
}

////////////////////////////////////////////////////////////////////////////////

/// Converts a CGAL mesh to a coolfluid mesh
template<typename TriangulationComplexT>
void cgal_to_coolfluid(const TriangulationComplexT& complex, Mesh& mesh)
{
  // Map CGAL vertex handles to coolfluid point indices
  typedef ::CGAL::Unique_hash_map<typename TriangulationComplexT::Vertex_handle,Uint,::CGAL::Handle_hash_function> VertexMapT;
  VertexMapT vertex_map(0, complex.number_of_cells()); // estimate the number of vertices equal to the cell count

  const Uint nb_cells = complex.number_of_cells();
  
  std::vector<Point> points; points.reserve(nb_cells*2);
  std::vector<Uint> cells; cells.reserve(nb_cells*4);

  for(typename TriangulationComplexT::Cell_iterator cell = complex.cells_begin(); cell != complex.cells_end(); ++cell) {
    for(Uint i = 0; i != 4; ++i) {
      typename TriangulationComplexT::Vertex_handle v = cell->vertex(i);
      if(!vertex_map.is_defined(v)) { // store the point, if we didn't have it already
        vertex_map[v] = points.size();
        points.push_back(v->point());
      }
      cells.push_back(vertex_map[v]);
    }
  }

  // Coordinates
  const Uint nb_points = points.size();
  Region& region = mesh.topology().create_region("region");
  Dictionary& nodes = mesh.geometry_fields();
  mesh.initialize_nodes(nb_points,DIM_3D);
  Field& coordinates = nodes.coordinates();
  for(Uint i = 0 ; i != nb_points; ++i)
  {
    Field::Row coords_row = coordinates[i];
    coords_row[XX] = ::CGAL::to_double(points[i].x());
    coords_row[YY] = ::CGAL::to_double(points[i].y());
    coords_row[ZZ] = ::CGAL::to_double(points[i].z());
  }

  // Connectivity
  Handle<Cells> tetras = region.create_component<Cells>("Tetras");
  tetras->initialize("cf3.mesh.LagrangeP1.Tetra3D",nodes);
  tetras->resize(cells.size());
  Table<Uint>& connectivity = tetras->geometry_space().connectivity();
  for(Uint i = 0; i != nb_cells; ++i)
  {
    Table<Uint>::Row row = connectivity[i];
    for(Uint j = 0; j != 4; ++j)
    {
      row[j] = cells[i*4 + j];
    }
  }
  
  List<Uint>& gids = mesh.geometry_fields().glb_idx(); gids.resize(nb_points);
  List<Uint>& ranks = mesh.geometry_fields().rank(); ranks.resize(nb_points);
  for(Uint i = 0; i != nb_points; ++i)
  {
    ranks[i] = 0;
    gids[i] = i;
  }
  
  mesh.raise_mesh_loaded();
}

////////////////////////////////////////////////////////////////////////////////

// Implementation is heavily based on the CGAL example
void create_mesh(const ImplicitFunction& function, Mesh& mesh, const MeshParameters parameters) {
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
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
