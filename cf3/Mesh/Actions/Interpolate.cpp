// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/OptionComponent.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "Mesh/Actions/Interpolate.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/ShapeFunction.hpp"
#include "Mesh/COcttree.hpp"

#include "Math/Consts.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
namespace Actions {

using namespace common;
using namespace common::PE;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Interpolate, CMeshTransformer, LibActions> Interpolate_Builder;

//////////////////////////////////////////////////////////////////////////////

Interpolate::Interpolate( const std::string& name )
: CMeshTransformer(name)
{

  properties()["brief"] = std::string("Interpolate Fields with matching support");
  std::string desc;
  desc =
    "Shapefunction/Space of the source-field is used to interpolate to the target";
  properties()["description"] = desc;

  m_options.add_option(OptionComponent<Field const>::create("source", &m_source))
      ->description("Field to interpolate from")
      ->pretty_name("Source Field")
      ->mark_basic();

  m_options.add_option(OptionComponent<Field>::create("target", &m_target))
      ->description("Field to interpolate to")
      ->pretty_name("TargetField")
      ->mark_basic();

  regist_signal ( "interpolate" )
      ->description( "Interpolate to given coordinates, not mesh-related" )
      ->pretty_name("Interpolate" )
      ->connect   ( boost::bind ( &Interpolate::signal_interpolate,    this, _1 ) )
      ->signature ( boost::bind ( &Interpolate::signature_interpolate, this, _1 ) );

}

/////////////////////////////////////////////////////////////////////////////

void Interpolate::execute()
{
  if (m_source.expired())
    throw SetupError(FromHere(),uri().string()+": Source field not configured");

  if (m_target.expired())
    throw SetupError(FromHere(),uri().string()+": Target field not configured");

  const Field& source = *m_source.lock();
  Field& target = *m_target.lock();

  if (source.row_size() != target.row_size())
    throw BadValue(FromHere(),"Field "+source.uri().string()+" has "+to_str(source.row_size())+" variables.\n"
                              "Field "+target.uri().string()+" has "+to_str(target.row_size())+" variables.");
  const Uint nb_vars(source.row_size());

  if(find_parent_component_ptr<CMesh>(source) == find_parent_component_ptr<CMesh>(target))
  {
    /// Loop over Regions
    boost_foreach(CEntities& elements, target.entities_range())
    {
      if (source.elements_lookup().contains(elements) == false)
        continue;
      //      throw BadValue(FromHere(),"Source field "+source.uri().string()+" is not defined in elements "+elements.uri().string());

      const CSpace& s_space = source.space(elements);
      const CSpace& t_space = target.space(elements);
      const ShapeFunction& s_sf = s_space.shape_function();
      const ShapeFunction& t_sf = t_space.shape_function();

      /// Compute Interpolation matrix, equal for every element
      RealMatrix interpolate(t_sf.nb_nodes(),s_sf.nb_nodes());
      for (Uint t_pt = 0; t_pt<t_sf.nb_nodes(); ++t_pt)
        interpolate.row(t_pt) = s_sf.value(t_sf.local_coordinates().row(t_pt));

      /// Element loop
      for (Uint e=0; e<elements.size(); ++e)
      {
        CConnectivity::ConstRow s_field_indexes = s_space.indexes_for_element(e);
        CConnectivity::ConstRow t_field_indexes = t_space.indexes_for_element(e);

        /// Interpolate: target[element] = interpolate * source[element]
        /// Split in loops since we cannot work with Matrix-products
        for (Uint t_pt=0; t_pt<t_sf.nb_nodes(); ++t_pt)
        {
          for (Uint var=0; var<nb_vars; ++var)
            target[t_field_indexes[t_pt]][var] = 0.;

          for (Uint s_pt=0; s_pt<s_sf.nb_nodes(); ++s_pt)
          {
            for (Uint var=0; var<nb_vars; ++var)
              target[t_field_indexes[t_pt]][var] += interpolate(t_pt,s_pt) * source[s_field_indexes[s_pt]][var];
          }
        }
      }
    }
  }
  else // fields are on different meshes
  {
    if (target.field_group().has_coordinates() == false)
      target.field_group().create_coordinates();
    interpolate(source,target.coordinates(),target);
  }
}

//////////////////////////////////////////////////////////////////////////////

void Interpolate::interpolate(const Field& source, const CTable<Real>& coordinates, CTable<Real>& target)
{

  if (Field::Ptr target_field = target.as_ptr<Field>())
  {
    if (source.row_size() != target_field->row_size())
      throw BadValue(FromHere(),"Field "+source.uri().string()+" has "+to_str(source.row_size())+" variables.\n"
                                "Field "+target.uri().string()+" has "+to_str(target.row_size())+" variables.");
    if (coordinates.size() != target.size())
      throw BadValue(FromHere(),"Field "+target.uri().string()+" has not the same number of rows as "+to_str(coordinates.size()));
  }
  else
  {
    target.set_row_size(source.row_size());
    target.resize(coordinates.size());
  }

  const CMesh& source_mesh = find_parent_component<CMesh>(source);

  if ( is_null(m_octtree) )
    m_octtree = create_component_ptr<COcttree>("octtree");

  if ( m_octtree->option("mesh").value<URI>().string() != source_mesh.uri().string() )
  {
    m_octtree->configure_option("mesh",source_mesh.uri());
    m_octtree->create_octtree();
  }
  const Uint dimension = source_mesh.dimension();
  const Uint nb_vars = source.row_size();
  m_source_space = source.field_group().space();
  m_source = source.as_ptr<Field>();

  CElements::ConstPtr element_component;
  Uint element_idx;
  std::deque<Uint> missing_cells;

  RealVector coord(dimension); coord.setZero();
  const Uint target_dim = coordinates.row_size();

  for(Uint i=0; i<coordinates.size(); ++i)
  {
    for (Uint d=0; d<target_dim; ++d)
      coord[d] = coordinates[i][d];
    if( m_octtree->find_element(coord,element_component,element_idx) )
    {
      cf3_assert(is_not_null(element_component));
      interpolate_coordinate( coord, *element_component, element_idx, target[i] );
//      std::cout<< PERank << "interpolate for coord (" << coord.transpose() << ") in " << element_component->uri().path() << "["<<element_idx<<"] ... done" << std::endl;
    }
    else
    {
//      std::cout << PERank<< "coord " << coord.transpose() << " not found " << std::endl;
      for (Uint v=0; v<nb_vars; ++v)
        target[i][v] = Math::Consts::real_max();
      missing_cells.push_back(i);
    }
  }

  std::vector<Real> send_coords(target_dim*missing_cells.size());
  std::vector<Real> recv_coords;

  Uint c(0);
  boost_foreach(const Uint i, missing_cells)
  {
    for(Uint d=0; d<target_dim; ++d)
      send_coords[c++]=coordinates[i][d];
  }

  for (Uint root=0; root<PE::Comm::instance().size(); ++root)
  {

//    if (root == PE::Comm::instance().rank())
//    {
//      std::cout << PERank << "send coords = ";
//      for(Uint i=0; i<send_coords.size(); ++i)
//         std::cout << send_coords[i] << " ";
//      std::cout << std::endl;
//    }
    recv_coords.resize(0);
    PE::Comm::instance().broadcast(send_coords,recv_coords,root,target_dim);

//    if (root != PE::Comm::instance().rank())
//    {
//      std::cout << PERank << "recv coords = ";
//      for(Uint i=0; i<recv_coords.size(); ++i)
//         std::cout << recv_coords[i] << " ";
//      std::cout << std::endl;
//    }


    // size is only because it doesn't get resized for this rank
    std::vector<Real> send_target_rows(nb_vars*missing_cells.size(),Math::Consts::uint_max());

    if (root!=Comm::instance().rank())
    {
      std::vector<RealVector> recv_coordinates(recv_coords.size()/target_dim) ;
      boost_foreach(RealVector& realvec, recv_coordinates)
          realvec.resize(target_dim);

      c=0;
      for (Uint i=0; i<recv_coordinates.size(); ++i)
      {
        for(Uint d=0; d<target_dim; ++d)
          recv_coordinates[i][d]=recv_coords[c++];
      }

      send_target_rows.resize(recv_coordinates.size()*nb_vars);
      for (Uint i=0; i<recv_coordinates.size(); ++i)
      {
        for (Uint d=0; d<target_dim; ++d)
          coord[d] = recv_coordinates[i][d];

        if( m_octtree->find_element(coord,element_component,element_idx) )
        {
//          std::cout<< PERank << " send to " << root << ": interpolate for coord (" << coord.transpose() << ") in " << element_component->uri().path() << "["<<element_idx<<"]" << std::endl;
          boost::multi_array<Real,2> target_row(boost::extents[1][nb_vars]);
          interpolate_coordinate( coord, *element_component, element_idx, target_row[0] );
          for (Uint v=0; v<nb_vars; ++v)
            send_target_rows[i*nb_vars+v] = target_row[0][v];
        }
        else
        {
//          std::cout << PERank<< " send to " << root << ": coord " << coord.transpose() << " not found " << std::endl;
          for (Uint v=0; v<nb_vars; ++v)
            send_target_rows[i*nb_vars+v] = Math::Consts::real_max();
        }
      }
    }

    std::vector<Real> recv_target_rows(nb_vars*missing_cells.size()*Comm::instance().size());
    PE::Comm::instance().gather(send_target_rows,recv_target_rows,root);

    if( root==Comm::instance().rank())
    {
      const Uint nb_cells = missing_cells.size();
      for (Uint i=0; i<missing_cells.size(); ++i)
      {
        for(Uint p=0; p<Comm::instance().size(); ++p)
        {
          for(Uint v=0; v<nb_vars;  ++v)
            target[missing_cells[i]][v] = std::min(recv_target_rows[v+nb_vars*(i+p*nb_cells)] , target[missing_cells[i]][v]);
        }
      }
    } // endif root==rank
  } // end foreach root


  missing_cells.clear();
  for (Uint i=0; i<target.size(); ++i)
  {
    bool found = true;
    for (Uint v=0; v<target.row_size(); ++v)
    {
      if (target[i][v] == Math::Consts::uint_max())
      {
        target[i][v] = 0.;
        found &= false;
      }
    }
    if (!found)
      missing_cells.push_back(i);
  }
  if(missing_cells.size())
  {
    std::cout << PERank << "could not interpolate " << missing_cells.size() << " coordinates, most likely due to issue#130, or point not on source. Values set to zero for coordinates : ";
    for(Uint i=0; i<missing_cells.size(); ++i)
    {
      for(Uint d=0; d<target_dim; ++d)
        std::cout << coordinates[missing_cells[i]] << "  ";
    }
    std::cout << std::endl;
  }

}

//////////////////////////////////////////////////////////////////////////////

void Interpolate::interpolate_coordinate(const RealVector& target_coord, const CElements& element_component, const Uint element_idx, Field::Row target_row)
{
  cf3_assert(m_source.expired() == false);
  const Field& source = *m_source.lock();
  const CSpace& source_space = element_component.space(m_source_space);
  const ShapeFunction& sf = source_space.shape_function();

  RealMatrix source_nodes(element_component.element_type().nb_nodes(),element_component.element_type().dimension());
  element_component.put_coordinates(source_nodes,element_idx);
  RealVector local_coord(sf.dimensionality());
  element_component.element_type().compute_mapped_coordinate(target_coord,source_nodes,local_coord);
  RealRowVector sf_value(sf.nb_nodes());
  sf.compute_value(local_coord,sf_value);

  CConnectivity::ConstRow source_indexes = source_space.indexes_for_element(element_idx);
  for(Uint v=0; v<target_row.size(); ++v)
  {
    target_row[v]=0.;
    for(Uint i=0; i<source_indexes.size(); ++i)
    {
      target_row[v] += source[source_indexes[i]][v] * sf_value[i];
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

void Interpolate::signal_interpolate ( common::SignalArgs& node )
{
  common::XML::SignalOptions options( node );

  URI source_uri = options.value<URI>("source");
  URI target_uri = options.value<URI>("target");
  URI coordinates_uri = options.value<URI>("coordinates");

  Field& source = access_component(source_uri).as_type<Field>();
  CTable<Real>& target = access_component(target_uri).as_type< CTable<Real> >();

  CTable<Real>::Ptr coordinates;
  if (coordinates_uri.string() == URI().string())
  {
    if ( Field::Ptr target_field = target.as_ptr<Field>() )
    {
      if (target_field->field_group().has_coordinates() == false)
        target_field->field_group().create_coordinates();
      coordinates = target_field->field_group().coordinates().as_ptr< CTable<Real> >();
    }
    else
    {
      throw SetupError(FromHere(),"argument \"coordinates\" not passed to interpolate() in "+uri().string());
    }
  }
  else
  {
    coordinates = access_component(coordinates_uri).as_ptr< CTable<Real> >();
  }

  interpolate(source,*coordinates,target);

}

//////////////////////////////////////////////////////////////////////////////

void Interpolate::signature_interpolate ( common::SignalArgs& node)
{
  common::XML::SignalOptions options( node );

  options.add_option< OptionURI >("source",URI())
      ->description("Source field")
      ->cast_to<OptionURI>()->supported_protocol( URI::Scheme::CPATH );

  options.add_option< OptionURI >("target",URI())
      ->description("Target field or table")
      ->cast_to<OptionURI>()->supported_protocol( URI::Scheme::CPATH );

  options.add_option< OptionURI >("coordinates",URI())
      ->description("Table of coordinates if target is not a field")
      ->cast_to<OptionURI>()->supported_protocol( URI::Scheme::CPATH );

}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // cf3
