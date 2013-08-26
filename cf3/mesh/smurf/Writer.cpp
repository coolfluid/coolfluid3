// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <set>
#include <boost/lexical_cast.hpp>
#include "common/BoostFilesystem.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"
#include "common/List.hpp"

#include "mesh/smurf/Writer.hpp"
#include "mesh/GeoShape.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Functions.hpp"
#include "mesh/MeshMetadata.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;

namespace cf3 {
namespace mesh {
namespace smurf {

#define CF3_BREAK_LINE(f,x) { if( x+1 % 10) { f << "\n"; } }

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < smurf::Writer, MeshWriter, LibSmurf> smurfWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

Writer::Writer( const std::string& name )
: MeshWriter(name)
{

  options().add("cell_centred",true)
    .description("True if discontinuous fields are to be plotted as cell-centred fields");
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Writer::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".smurf");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write()
{
  boost::filesystem::path path(m_file_path.path());
  if (PE::Comm::instance().size() > 1)
    path = boost::filesystem::basename(path) + "_P" + to_str(PE::Comm::instance().rank()) + boost::filesystem::extension(path);

  SmURF::MeshWriter mwriter(path.string(), SmURF::DOUBLE, false, 107, m_mesh->metadata().properties().value<Real>("time")); // add solution time here if needed !


  // Get variable names
  std::vector< std::string > vn;

  Uint dimension = m_mesh->geometry_fields().coordinates().row_size();
  // coordinate variable names
  for (Uint i = 0; i < dimension ; ++i)
  {
    vn.push_back("x"+boost::lexical_cast<std::string>(i));
  }

  std::vector<Uint> cell_centered_var_ids;
  Uint zone_var_id(dimension);
  boost_foreach(Handle<Field const> field_ptr, m_fields)
  {
    const Field& field = *field_ptr;
    for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
    {
      VarType var_type = field.var_length(iVar);
      std::string var_name = field.var_name(iVar);

      if ( static_cast<Uint>(var_type) > 1)
      {
        for (Uint i=0; i<static_cast<Uint>(var_type); ++i)
        {
          vn.push_back(var_name);
          ++zone_var_id;
          if (field.discontinuous())
            cell_centered_var_ids.push_back(zone_var_id);
        }
      }
      else
      {
        vn.push_back(var_name);
        ++zone_var_id;
        if (field.discontinuous())
          cell_centered_var_ids.push_back(zone_var_id);
      }
    }
  }
  CFdebug << "smurf: writing main header" << CFendl;
  boost_foreach(std::string const& s, vn) CFdebug << "       var: " << s << CFendl;
  mwriter.writeMainHeader("COOLFluiD_Mesh_Data",vn);


  // loop over the element types
  // and create a zone in the tecplot file for each element type
  Uint zone_idx=0;
  boost_foreach (const Handle<Entities const>& elements_h, m_filtered_entities )
  {
    Entities const& elements = *elements_h;
    const ElementType& etype = elements.element_type();

    Uint nb_elems = elements.size();

    if(m_enable_overlap == false)
    {
      Uint nb_ghost_elems = 0;
      for (Uint e=0; e<nb_elems; ++e)
      {
        if (elements.is_ghost(e))
          ++nb_ghost_elems;
      }
      nb_elems -= nb_ghost_elems;
    }

    std::string zone_name = elements.parent()->uri().path();
    boost::algorithm::replace_first(zone_name,m_mesh->topology().uri().path()+"/","");
    std::set<std::string> zone_names;
    if (zone_names.count(zone_name) == 0)
    {
      zone_idx++;
      zone_names.insert(zone_name);
    }

    // tecplot doesn't handle zones with 0 elements
    // which can happen in parallel, so skip them
    if (nb_elems == 0) continue;

    if (etype.order() > 1)
    {
      throw NotImplemented(FromHere(), "Tecplot can only output P1 elements. A new P1 space should be created, and used as geometry space");
    }

    boost::shared_ptr< common::List<Uint> > used_nodes_ptr = mesh::build_used_nodes_list(elements,m_mesh->geometry_fields(),m_enable_overlap);
    common::List<Uint> const& used_nodes = *used_nodes_ptr;

    // print zone header,
    // one zone per element type per cpu
    // therefore the title is dependent on those parameters
    CFdebug << "smurf: writing zone header \"" << zone_name << "\"" << CFendl
            << "       etype   : " << zone_type(etype)  << CFendl
            << "       nb_nodes: " << used_nodes.size() << CFendl
            << "       nb_elems: " << nb_elems          << CFendl
            << "       zone_idx: " << zone_idx          << CFendl;
    mwriter.writeZoneHeader(zone_type(etype),SmURF::BLOCK,zone_name, used_nodes.size(),nb_elems, 1, zone_idx);

    if (cell_centered_var_ids.size() && options().value<bool>("cell_centred"))
    {
      // TODO: implement cell-centered vars
      CFerror << "Not implemented for cellcentered" << CFendl;
#if 0
      file << ",VARLOCATION=(["<<cell_centered_var_ids[0];
      for (Uint i=1; i<cell_centered_var_ids.size(); ++i)
        file << ","<<cell_centered_var_ids[i];
      file << "]=CELLCENTERED)";
#endif
    }
  }

  boost_foreach (const Handle<Entities const>& elements_h, m_filtered_entities )
  {
    Entities const& elements = *elements_h;
    const ElementType& etype = elements.element_type();

    Uint nb_elems = elements.size();

    if(m_enable_overlap == false)
    {
      Uint nb_ghost_elems = 0;
      for (Uint e=0; e<nb_elems; ++e)
      {
        if (elements.is_ghost(e))
          ++nb_ghost_elems;
      }
      nb_elems -= nb_ghost_elems;
    }

    // tecplot doesn't handle zones with 0 elements
    // which can happen in parallel, so skip them
    if (nb_elems == 0) continue;

    boost::shared_ptr< common::List<Uint> > used_nodes_ptr = mesh::build_used_nodes_list(elements,m_mesh->geometry_fields(),m_enable_overlap);
    common::List<Uint>& used_nodes = *used_nodes_ptr;
    std::map<Uint,Uint> zone_node_idx;
    for (Uint n=0; n<used_nodes.size(); ++n)
      zone_node_idx[ used_nodes[n] ] = n; // zero based for binary tecplot (as opposed to one-based for ascii tecplot!)


    std::vector< std::vector<double> > vv;
    vv.reserve(vn.size());

    // loop over coordinates
    const common::Table<Real>& coordinates = m_mesh->geometry_fields().coordinates();
    for (Uint d = 0; d < dimension; ++d)
    {
      std::vector<double> var; var.reserve(used_nodes.array().size());
      boost_foreach(Uint n, used_nodes.array())
      {
        cf3_assert(n<coordinates.size());
        var.push_back(coordinates[n][d]);
      }
      vv.push_back(var);
    }

    boost_foreach(Handle<Field const> field_ptr, m_fields)
    {
      const Field& field = *field_ptr;
      Uint var_idx(0);
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        VarType var_type = field.var_length(iVar);
        std::string var_name = field.var_name(iVar);

        for (Uint i=0; i<static_cast<Uint>(var_type); ++i)
        {
          if (field.continuous())
          {
            // Continuous field with different space than geometry
            if ( &field.dict() == &m_mesh->geometry_fields() )
            {
              std::vector<double> var; var.reserve(used_nodes.array().size());
              boost_foreach(Uint n, used_nodes.array())
              {
                var.push_back(field[n][var_idx]);
              }
              vv.push_back(var);
            }

            // Continuous field with different space than geometry
            else
            {
              if (field.dict().defined_for_entities(elements.handle<Entities>()) )
              {
                const Space& field_space = field.space(elements);
                RealVector field_data (field_space.shape_function().nb_nodes());

                std::vector<Real> nodal_data(used_nodes.size(),0.);

                RealMatrix interpolation(elements.geometry_space().shape_function().nb_nodes(),field_space.shape_function().nb_nodes());
                const RealMatrix& geometry_local_coords = elements.geometry_space().shape_function().local_coordinates();
                const ShapeFunction& sf = field_space.shape_function();
                for (Uint g=0; g<interpolation.rows(); ++g)
                {
                  interpolation.row(g) = sf.value(geometry_local_coords.row(g));
                }

                // Compute interpolated data in the vector nodal_data
                for (Uint e=0; e<elements.size(); ++e)
                {
                  // Skip this element if it is a ghost cell and overlap is disabled
                  if (m_enable_overlap || !elements.is_ghost(e))
                  {
                    // get the node indices of this element
                    Connectivity::ConstRow field_index = field_space.connectivity()[e];

                    /// set field data
                    for (Uint iState=0; iState<field_space.shape_function().nb_nodes(); ++iState)
                    {
                      field_data[iState] = field[field_index[iState]][var_idx];
                    }

                    /// evaluate field shape function in P0 space
                    RealVector geometry_field_data = interpolation*field_data;

                    Connectivity::ConstRow geom_nodes = elements.geometry_space().connectivity()[e];
                    cf3_assert(geometry_field_data.size()==geom_nodes.size());
                    /// Average nodal values
                    for (Uint g=0; g<geom_nodes.size(); ++g)
                    {
                      const Uint geom_node = geom_nodes[g];
                      const Uint node_idx = zone_node_idx[geom_node]-1;
                      cf3_assert(node_idx < nodal_data.size());
                      nodal_data[node_idx] = geometry_field_data[g];
                    }
                  }
                }
                vv.push_back(nodal_data);
              }
            }
          }
#if 0
          // Discontinuous fields
          else
          {
            if (field.dict().defined_for_entities(elements.handle<Entities>()))
            {
              const Space& field_space = field.space(elements);
              RealVector field_data (field_space.shape_function().nb_nodes());

              if (options().value<bool>("cell_centred"))
              {
                boost::shared_ptr< ShapeFunction > P0_cell_centred = boost::dynamic_pointer_cast<ShapeFunction>(build_component("cf3.mesh.LagrangeP0."+to_str(elements.element_type().shape_name()),"tmp_shape_func"));

                for (Uint e=0; e<elements.size(); ++e)
                {
                  if (m_enable_overlap || !elements.is_ghost(e))
                  {
                    Connectivity::ConstRow field_index = field_space.connectivity()[e];
                    /// set field data
                    for (Uint iState=0; iState<field_space.shape_function().nb_nodes(); ++iState)
                    {
                      field_data[iState] = field[field_index[iState]][var_idx];
                    }

                    /// get cell-centred local coordinates
                    RealVector local_coords = P0_cell_centred->local_coordinates().row(0);

                    /// evaluate field shape function in P0 space
                    Real cell_centred_data = field_space.shape_function().value(local_coords)*field_data;

                    /// Write cell centred value
                    file << cell_centred_data << " ";
                    CF3_BREAK_LINE(file,e);
                  }
                }
                file << "\n";
              }
              else
              {
                std::vector<Real> nodal_data(used_nodes.size(),0.);
                std::vector<Uint> nodal_data_count(used_nodes.size(),0u);

                RealMatrix interpolation(elements.geometry_space().shape_function().nb_nodes(),field_space.shape_function().nb_nodes());
                const RealMatrix& geometry_local_coords = elements.geometry_space().shape_function().local_coordinates();
                const ShapeFunction& sf = field_space.shape_function();
                for (Uint g=0; g<interpolation.rows(); ++g)
                {
                  interpolation.row(g) = sf.value(geometry_local_coords.row(g));
                }

                for (Uint e=0; e<elements.size(); ++e)
                {
                  Connectivity::ConstRow field_index = field_space.connectivity()[e];

                  /// set field data
                  for (Uint iState=0; iState<field_space.shape_function().nb_nodes(); ++iState)
                  {
                    field_data[iState] = field[field_index[iState]][var_idx];
                  }

                  /// evaluate field shape function in P0 space
                  RealVector geometry_field_data = interpolation*field_data;

                  Connectivity::ConstRow geom_nodes = elements.geometry_space().connectivity()[e];
                  cf3_assert(geometry_field_data.size()==geom_nodes.size());
                  /// Average nodal values
                  for (Uint g=0; g<geom_nodes.size(); ++g)
                  {
                    const Uint geom_node = geom_nodes[g];
                    if (zone_node_idx.find(geom_node) != zone_node_idx.end())
                    {
                      const Uint node_idx = zone_node_idx[geom_node]-1;
                      cf3_assert(node_idx < nodal_data.size());
                      const Real accumulated_weight = nodal_data_count[node_idx]/(nodal_data_count[node_idx]+1.0);
                      const Real add_weight = 1.0/(nodal_data_count[node_idx]+1.0);
                      nodal_data[node_idx] = accumulated_weight*nodal_data[node_idx] + add_weight*geometry_field_data[g];
                      ++nodal_data_count[node_idx];
                    }
                  }
                }

                for (Uint n=0; n<nodal_data.size(); ++n)
                {
                  file << nodal_data[n] << " ";
                    CF3_BREAK_LINE(file,n)
                }
                file << "\n";

              }
            }
            else
            {
              // field not defined for this zone, so write zeros
              if (options().value<bool>("cell_centred"))
                file << nb_elems << "*" << 0.;
              else
                file << used_nodes.size() << "*" << 0.;
            }
          }
#endif
          var_idx++;
        }
      }
    }

    // write connectivity
    const Connectivity& connectivity = elements.geometry_space().connectivity();
    std::vector<std::vector<unsigned> > ve; ve.reserve(connectivity.size());

    for (Uint e=0; e<elements.size(); ++e)
    {
      if (m_enable_overlap || !elements.is_ghost(e))
      {
        std::vector<unsigned> element; element.reserve(connectivity.row_size());
        boost_foreach ( Uint n, connectivity[e])
            element.push_back(zone_node_idx[n]);
        ve.push_back(element);
      }
    }

    if (etype.shape() == GeoShape::POINT) {

      // these are represented by a FELINESEG, with coalesced nodes
      for (unsigned c=0; c<ve.size(); ++c) {
        const std::vector< unsigned >  eng = ve[c];  // element nodes, original (gambit)
              std::vector< unsigned >& ent = ve[c];  // ...,           modified (tecplot)
        ent.resize(2);
        ent[0] = eng[0];
        ent[1] = eng[0];
      }

    }
    else if (etype.shape() == GeoShape::HEXA) { //*2

      // these are represented by a FEBRICK, with coalesced nodes
      for (unsigned c=0; c<ve.size(); ++c) {
        const std::vector< unsigned >  eng = ve[c];  // element nodes, original (gambit)
              std::vector< unsigned >& ent = ve[c];  // ...,           modified (tecplot)
        ent.resize(8);
        ent[0] = eng[0];
        ent[1] = eng[1];
        ent[2] = eng[2];
        ent[3] = eng[2];
        ent[4] = eng[3];
        ent[5] = eng[4];
        ent[6] = eng[5];
        ent[7] = eng[5];
      }

    }
    else if (etype.shape() == GeoShape::PYRAM) { //*3

      // these are represented by a FEBRICK, with coalesced nodes
      for (unsigned c=0; c<ve.size(); ++c) {
        const std::vector< unsigned >  eng = ve[c];  // element nodes, original (gambit)
              std::vector< unsigned >& ent = ve[c];  // ...,           modified (tecplot)
        ent.resize(8);
        ent[0] = eng[0];
        ent[1] = eng[1];
        ent[2] = eng[3];
        ent[3] = eng[2];
        ent[4] = eng[4];
        ent[5] = eng[4];
        ent[6] = eng[4];
        ent[7] = eng[4];
      }

    }
    else if (etype.shape() == GeoShape::HEXA) { //*1

      // these need renumbering
      for (unsigned c=0; c<ve.size(); ++c) {
        std::swap(ve[c][2],ve[c][3]);
        std::swap(ve[c][6],ve[c][7]);
      }

    }

    // TODO: use varsharelist? feasible with parallel?
    CFdebug << "smurf: writing variables and connectivities" << CFendl
            << "       etype   : " << zone_type(etype) << CFendl
            << "       nb_elems: " << ve.size() << CFendl
            << "       nb_vars : " << vv.size() << CFendl
            << "       share   : " << "disabled" << CFendl;
    mwriter.writeZoneData(zone_type(etype),SmURF::BLOCK,ve,vv, -1);
  }
}

/////////////////////////////////////////////////////////////////////////////

SmURF::ZoneType Writer::zone_type(const ElementType& etype) const
{
  return
      (etype.shape() == GeoShape::POINT? SmURF::FELINESEG       :
      (etype.shape() == GeoShape::LINE ? SmURF::FELINESEG       :
      (etype.shape() == GeoShape::TRIAG? SmURF::FETRIANGLE      :
      (etype.shape() == GeoShape::TETRA? SmURF::FETETRAHEDRON   :
      (etype.shape() == GeoShape::HEXA ? SmURF::FEBRICK         :
      (etype.shape() == GeoShape::QUAD ? SmURF::FEQUADRILATERAL :
      (etype.shape() == GeoShape::PRISM? SmURF::FEBRICK         :
      (etype.shape() == GeoShape::PYRAM? SmURF::FEBRICK         :
                                         SmURF::ORDERED ))))))));
}
////////////////////////////////////////////////////////////////////////////////

} // smurf
} // mesh
} // cf3
