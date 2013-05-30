// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <limits>
#include <set>

#include <boost/iterator/zip_iterator.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>

#include "common/Builder.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Functions.hpp"

#include "CGAL/Tetrahedralize.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Tetrahedralize, mesh::MeshTransformer, LibCGAL> Tetrahedralize_Builder;

////////////////////////////////////////////////////////////////////////////////

typedef ::CGAL::Exact_predicates_inexact_constructions_kernel K;
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

  typedef ::CGAL::Unique_hash_map<Vertex_handle,Uint,::CGAL::Handle_hash_function> VertexMapT;
  VertexMapT vertex_map(0, nb_points);
  std::vector<Vertex_handle> vertex_handles;

  Triangulation triangulation;

  std::vector< Handle<common::Component> > to_remove;
  BOOST_FOREACH(mesh::Elements& elems, common::find_components_recursively_with_filter<mesh::Elements>(mesh(), mesh::IsElementsVolume()))
  {
//     const mesh::Connectivity& conn = elems.geometry_space().connectivity();
//     const Uint nb_elems = conn.size();
//     const Uint nb_elem_points = conn.row_size();
//     for(Uint i = 0; i != nb_elems; ++i)
//     {
//       const mesh::Connectivity::ConstRow conn_row = conn[i];
//       for(Uint j = 0; j != nb_elem_points; ++j)
//       {
//         vertex_map[triangulation.insert(detail::to_point(coordinates[conn_row[j]]))] = conn_row[j];
//       }
//     }
    
    boost::shared_ptr< common::List<Uint> > used_nodes = mesh::build_used_nodes_list(elems, mesh().geometry_fields(), true, false);
    std::vector< Point > used_points; used_points.reserve(used_nodes->size());
    BOOST_FOREACH(const Uint i, used_nodes->array())
    {
      used_points.push_back(detail::to_point(coordinates[i]));
    }

    triangulation.insert(boost::make_zip_iterator(boost::make_tuple(used_points.begin(), used_nodes->array().begin())),
      boost::make_zip_iterator(boost::make_tuple(used_points.end(), used_nodes->array().end())));
    to_remove.push_back(elems.handle());
  }

  cf3_assert(triangulation.is_valid());
  cf3_assert(nb_points == triangulation.number_of_vertices());

  // Connectivity
  mesh::Region& region = mesh().topology().create_region("tetrahedra");
  Handle<mesh::Cells> tetras = region.create_component<mesh::Cells>("Tetras");
  tetras->initialize("cf3.mesh.LagrangeP1.Tetra3D",mesh().geometry_fields());
  tetras->resize(triangulation.number_of_finite_cells());
  common::Table<Uint>& connectivity = tetras->geometry_space().connectivity();

  Uint cell_number = 0;
  for(Triangulation::Finite_cells_iterator cell_it = triangulation.finite_cells_begin(); cell_it != triangulation.finite_cells_end(); ++cell_it)
  {
//     ::CGAL::Tetrahedron_3<K> tet(cell_it->vertex(0)->point(), cell_it->vertex(1)->point(), cell_it->vertex(2)->point(), cell_it->vertex(3)->point());
//     if(tet.volume() < 1e-14)
//       continue;
    common::Table<Uint>::Row conn_row = connectivity[cell_number];
    for(int i = 0; i != 4; ++i)
    {
      conn_row[i] = cell_it->vertex(i)->info();
    }

    ++cell_number;
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
