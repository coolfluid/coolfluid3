// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <limits>

#include <boost/iterator/zip_iterator.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Filtered_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

#include "common/Builder.hpp"
#include "common/List.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Mesh.hpp"

#include "CGAL/Tetrahedralize.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Tetrahedralize, mesh::MeshTransformer, LibCGAL> Tetrahedralize_Builder;

////////////////////////////////////////////////////////////////////////////////

typedef ::CGAL::Filtered_kernel< ::CGAL::Simple_cartesian<long> > K;
typedef ::CGAL::Triangulation_vertex_base_with_info_3<Uint, K>    Vb;
typedef ::CGAL::Triangulation_data_structure_3<Vb>                    Tds;

typedef ::CGAL::Delaunay_triangulation_3<K, Tds>      Triangulation;

typedef Triangulation::Cell_handle    Cell_handle;
typedef Triangulation::Vertex_handle  Vertex_handle;
typedef Triangulation::Locate_type    Locate_type;
typedef Triangulation::Point          Point;

////////////////////////////////////////////////////////////////////////////////

namespace detail
{
  /// Convert a floating point value to integer
  long to_long(const Real r)
  {
    static const Real multiplier = 1e9;
    static const Real max_val = std::numeric_limits< long >::max();
    static const Real min_val = std::numeric_limits< long >::min();
    const Real multiplied = r * multiplier;
    cf3_assert(multiplied < max_val);
    cf3_assert(multiplied > min_val);
    return round(multiplied);
  }
  
  /// Helper function to convert coordinates to integers
  template<typename CoordT>
  Point to_point(const CoordT& coord)
  {
    return Point(to_long(coord[XX]), to_long(coord[YY]), to_long(coord[ZZ]));
  }
}

Tetrahedralize::Tetrahedralize ( const std::string& name ) : MeshTransformer ( name )
{
}

Tetrahedralize::~Tetrahedralize()
{
}

void Tetrahedralize::execute()
{
  const mesh::Field& coordinates = mesh().geometry_fields().coordinates();
  const Uint nb_points = coordinates.size();

  std::vector< Handle<common::Component> > to_remove;
  BOOST_FOREACH(mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(mesh(), mesh::IsElementsVolume()))
  {
    to_remove.push_back(elems.handle());
  }

  std::vector<Point> cgal_points; cgal_points.reserve(nb_points);
  std::vector<Uint> node_indices(nb_points);
  for(Uint i = 0; i != nb_points; ++i)
  {
    cgal_points.push_back(detail::to_point(coordinates[i]));
    node_indices[i] = i;
  }

  Triangulation triangulation;
  triangulation.insert(boost::make_zip_iterator(boost::make_tuple(cgal_points.begin(), node_indices.begin())),
      boost::make_zip_iterator(boost::make_tuple(cgal_points.end(), node_indices.end())));
  
  cf3_assert(triangulation.is_valid());
  cf3_assert(nb_points == triangulation.number_of_vertices());
  
  mesh::Region& region = mesh().topology().create_region("tetras");
  Handle<mesh::Cells> tetras = region.create_component<mesh::Cells>("Tetras");
  tetras->initialize("cf3.mesh.LagrangeP1.Tetra3D",mesh().geometry_fields());
  tetras->resize(triangulation.number_of_finite_cells());
  common::Table<Uint>& connectivity = tetras->geometry_space().connectivity();
  Uint cell_idx = 0;
  for(Triangulation::Finite_cells_iterator cell_it = triangulation.finite_cells_begin(); cell_it != triangulation.finite_cells_end(); ++cell_it)
  {
    mesh::Connectivity::Row conn_row = connectivity[cell_idx];
    for(Uint i = 0; i != 4; ++i)
    {
      conn_row[i] = cell_it->vertex(i)->info();
    }
    ++cell_idx;
  }

  BOOST_FOREACH(const Handle<common::Component>& comp, to_remove)
  {
    comp->parent()->remove_component(*comp);
  }
}


////////////////////////////////////////////////////////////////////////////////

} // namespace CGAL
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
