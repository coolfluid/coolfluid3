// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Signal.hpp"
#include "common/PE/Comm.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Field.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Quadrature.hpp"
#include "mesh/Connectivity.hpp"

#include "mesh/actions/SurfaceIntegral.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

using namespace common;
using namespace common::PE;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < SurfaceIntegral, Action, mesh::actions::LibActions> SurfaceIntegral_Builder;

//////////////////////////////////////////////////////////////////////////////

SurfaceIntegral::SurfaceIntegral( const std::string& name )
: Action(name)
{

  properties()["brief"] = std::string("Compute surface-integral of field");
  std::string desc;
  desc =
    "Compute surface integral of a field, given surface regions";
  properties()["description"] = desc;

  m_order = 2.;
  options().add("order", m_order)
      .description("order of integration")
      .pretty_name("Order")
      .mark_basic()
      .link_to(&m_order);

  options().add("field", m_field)
      .description("Field to integrate")
      .pretty_name("Field")
      .mark_basic()
      .link_to(&m_field);

  options().add("regions", m_regions)
      .description("Regions to integrate")
      .pretty_name("Regions")
      .mark_basic()
      .link_to(&m_regions);

  regist_signal ( "integrate" )
      .description( "SurfaceIntegral" )
      .pretty_name("SurfaceIntegral" )
      .connect   ( boost::bind ( &SurfaceIntegral::signal_integrate,    this, _1 ) )
      .signature ( boost::bind ( &SurfaceIntegral::signature_integrate, this, _1 ) );

}

/////////////////////////////////////////////////////////////////////////////

void SurfaceIntegral::execute()
{
  if (is_null(m_field))
    throw SetupError(FromHere(),uri().string()+": field not configured");

  if (m_regions.empty())
    throw SetupError(FromHere(),uri().string()+": regions not configured");
  
  integrate(*m_field, m_regions);
}

//////////////////////////////////////////////////////////////////////////////

Real SurfaceIntegral::integrate(const Field& field, const std::vector< Handle<Region> >& regions)
{
  // Create a set, so that we have no duplicate entities
  std::set< Handle<Entities> > entities_set;
  boost_foreach( const Handle<Region>& region, regions)
  {
    boost_foreach( Entities& patch, find_components_recursively<Entities>(*region) )
    {
      // only surface elements are added
      if( patch.element_type().dimensionality() == patch.element_type().dimension()-1 )
        entities_set.insert(patch.handle<Entities>());
    }
  }

  // Transform the set to a vector
  std::vector< Handle<Entities> > entities; entities.reserve(entities_set.size());
  boost_foreach( const Handle<Entities>& patch, entities_set)
  {
    entities.push_back(patch);
  }
  return integrate(field, entities);
}

Real SurfaceIntegral::integrate(const Field& field, const std::vector< Handle<Entities> >& entities)
{
  Real local_integral = 0.;
  boost_foreach( const Handle<Entities>& patch, entities )
  {
    if( patch->element_type().dimensionality() >= patch->element_type().dimension() )
      throw SetupError( FromHere(), "Cannot compute surface integral of volume element");

    if( is_not_null(m_quadrature) )
      remove_component("quadrature");
    m_quadrature = create_component<Quadrature>("quadrature",
        "cf3.mesh.gausslegendre."+GeoShape::Convert::instance().to_str(patch->element_type().shape())+"P"+common::to_str(m_order));

    /// Common part for every element of this patch
    const Space& space = field.space(*patch);
    const Uint nb_elems = space.size();
    const Uint nb_nodes_per_elem = space.shape_function().nb_nodes();
    const Uint nb_qdr_pts = m_quadrature->nb_nodes();
    const Uint nb_vars = field.row_size();

    RealMatrix qdr_pt_values( nb_qdr_pts, nb_vars );
    RealMatrix interpolate( nb_qdr_pts, nb_nodes_per_elem );
    RealMatrix field_pt_values( nb_nodes_per_elem, nb_vars );
    RealMatrix elem_coords;

    patch->geometry_space().allocate_coordinates(elem_coords);

    for( Uint qn=0; qn<nb_qdr_pts; ++qn)
    {
      interpolate.row(qn) = space.shape_function().value( m_quadrature->local_coordinates().row(qn) );
    }

    /// Loop over every element of this patch
    for (Uint e=0; e<nb_elems; ++e)
    {
      if( ! patch->is_ghost(e) )
      {
        patch->geometry_space().put_coordinates(elem_coords,e);

        // interpolate
        for (Uint n=0; n<nb_nodes_per_elem; ++n)
        {
          const Uint p = space.connectivity()[e][n];
          for (Uint v=0; v<nb_vars; ++v)
          {
            field_pt_values(n,v) = field[p][v];
          }
        }
        qdr_pt_values = interpolate * field_pt_values;

        // integrate
        for( Uint qn=0; qn<nb_qdr_pts; ++qn)
        {
          const Real Jdet = patch->element_type().jacobian_determinant( m_quadrature->local_coordinates().row(qn), elem_coords );
          local_integral += Jdet * m_quadrature->weights()[qn] * qdr_pt_values(qn,0);
        }
      }
    }
  }
  Real global_integral;
  PE::Comm::instance().all_reduce(PE::plus(), &local_integral, 1, &global_integral);
  return global_integral;
}

//////////////////////////////////////////////////////////////////////////////

void SurfaceIntegral::signal_integrate ( common::SignalArgs& node )
{
  common::XML::SignalOptions options( node );

  Field& field = *options.value< Handle<Field> >("field");
  std::vector< Handle<Region> > regions = options.value< std::vector<Handle<Region> > >("regions");

  Real integral = integrate(field,regions);

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("return_value", integral);
}

//////////////////////////////////////////////////////////////////////////////

void SurfaceIntegral::signature_integrate ( common::SignalArgs& node)
{
  common::XML::SignalOptions options( node );

  options.add("field", Handle<Field>() )
      .description("Field to integrate");

  options.add("regions",std::vector< Handle<Region> >())
      .description("Regions to integrate");
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
