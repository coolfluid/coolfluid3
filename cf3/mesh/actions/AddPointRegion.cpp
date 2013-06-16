// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "AddPointRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < AddPointRegion, MeshTransformer, mesh::actions::LibActions> AddPointRegion_Builder;

////////////////////////////////////////////////////////////////////////////////

AddPointRegion::AddPointRegion(const std::string& name) : MeshTransformer(name)
{
  options().add("coordinates", std::vector<Real>())
      .pretty_name("Coordinates")
      .description("Coordinates of the point to turn into a region. The closest point to the supplied coordinates will be chosen")
      .mark_basic();

  options().add("region_name", "PointRegion")
      .pretty_name("Region Name")
      .description("Name of the region to create")
      .mark_basic();
}

void AddPointRegion::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();

  Mesh& mesh = *m_mesh;
  const Field& coords = mesh.geometry_fields().coordinates();
  Real my_min_dist2 = 1e30;
  Uint my_i = 0;
  const Uint nb_points = coords.size();

  const RealVector target_point = to_vector(options().value< std::vector<Real> >("coordinates"));
  if(target_point.rows() != mesh.dimension())
    throw common::SetupError(FromHere(), "Dimension of target coordinates does not match mesh dimension");

  for(Uint i = 0; i != nb_points; ++i)
  {
    const Real dist2 = (target_point - to_vector(coords[i])).squaredNorm();
    if(dist2 < my_min_dist2)
    {
      my_i = i;
      my_min_dist2 = dist2;
    }
  }

  Real min_dist2;
  if(comm.size() > 1)
  {
    comm.all_reduce(common::PE::min(), &my_min_dist2, 1, &min_dist2);
  }
  else
  {
    min_dist2 = my_min_dist2;
  }

  // Get the maximum GID that is in use for the mesh
  Uint my_max_gid = 0;
  boost_foreach(const Entities& entities, common::find_components_recursively<Entities>(mesh))
  {
    boost_foreach(const Uint gid, entities.glb_idx().array())
    {
      if(gid > my_max_gid)
        my_max_gid = gid;
    }
  }

  Uint max_gid;

  if(comm.size() > 1)
  {
    comm.all_reduce(common::PE::max(), &my_max_gid, 1, &max_gid);
  }
  else
  {
    max_gid = my_max_gid;
  }

  const std::string region_name = options().value<std::string>("region_name");
  Handle<Elements> point_elems = mesh.topology().create_region(region_name).create_component<Elements>("Point");
  mesh.topology().get_child(region_name)->mark_basic();
  point_elems->initialize("cf3.mesh.LagrangeP0.Point2D",mesh.geometry_fields());
  if(min_dist2 == my_min_dist2)
  {
    point_elems->resize(1);
    common::Table<Uint>& connectivity = point_elems->geometry_space().connectivity();
    connectivity[0][0] = my_i;
    point_elems->glb_idx()[0] = max_gid + 1;
    point_elems->rank()[0] = mesh.geometry_fields().rank()[my_i];
  }
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
