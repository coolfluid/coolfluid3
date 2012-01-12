// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "common/BoostFilesystem.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"

#include "mesh/tecplot/Writer.hpp"
#include "mesh/GeoShape.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Space.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;

namespace cf3 {
namespace mesh {
namespace tecplot {

#define CF3_BREAK_LINE(f,x) { if( x+1 % 10) { f << "\n"; } }

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < tecplot::Writer, MeshWriter, LibTecplot> atecplotWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

Writer::Writer( const std::string& name )
: MeshWriter(name)
{

  options().add_option("cell_centred",false)
    .description("True if discontinuous fields are to be plotted as cell-centred fields");
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Writer::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".plt");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void Writer::write_from_to(const Mesh& mesh, const URI& file_path)
{
  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path(file_path.path());
  if (PE::Comm::instance().size() > 1)
  {
    path = boost::filesystem::basename(path) + "_P" + to_str(PE::Comm::instance().rank()) + boost::filesystem::extension(path);
  }
//  CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }


  write_file(file, mesh);


  file.close();

}
/////////////////////////////////////////////////////////////////////////////

void Writer::write_file(std::fstream& file, const Mesh& mesh)
{
  file << "TITLE      = COOLFluiD Mesh Data" << "\n";
  file << "VARIABLES  = ";

  Uint dimension = mesh.geometry_fields().coordinates().row_size();
  // write the coordinate variable names
  for (Uint i = 0; i < dimension ; ++i)
  {
    file << " \"x" << i << "\" ";
  }

  std::vector<Uint> cell_centered_var_ids;
  Uint zone_var_id(dimension);
  boost_foreach(Handle<Field> field_ptr, m_fields)
  {
    Field& field = *field_ptr;
    for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
    {
      Field::VarType var_type = field.var_length(iVar);
      std::string var_name = field.var_name(iVar);

      if ( static_cast<Uint>(var_type) > 1)
      {
        for (Uint i=0; i<static_cast<Uint>(var_type); ++i)
        {
          file << " \"" << var_name << "["<<i<<"]\"";
          ++zone_var_id;
          if (field.basis() != SpaceFields::Basis::POINT_BASED)
            cell_centered_var_ids.push_back(zone_var_id);
        }
      }
      else
      {
        file << " \"" << var_name <<"\"";
        ++zone_var_id;
        if (field.basis() != SpaceFields::Basis::POINT_BASED)
          cell_centered_var_ids.push_back(zone_var_id);
      }
    }
  }
  file << "\n";


  // loop over the element types
  // and create a zone in the tecplot file for each element type
  std::map<Handle<Component const>,Uint> zone_id;
  Uint zone_idx=0;
  boost_foreach (const Elements& elements, find_components_recursively<Elements>(mesh.topology()) )
  {
    const ElementType& etype = elements.element_type();
    if (etype.shape() == GeoShape::POINT)
      continue;
    // tecplot doesn't handle zones with 0 elements
    // which can happen in parallel, so skip them
    if (elements.size() == 0)
      continue;

    if (etype.order() != 1)
    {
      throw NotImplemented(FromHere(), "Tecplot can only output P1 elements. A new P1 space should be created, and used as geometry space");
    }

    zone_id[elements.handle<Component>()] = zone_idx++;

    boost::shared_ptr< common::List<Uint> > used_nodes_ptr = Entities::create_used_nodes(elements,mesh::Tags::geometry());
    common::List<Uint>& used_nodes = *used_nodes_ptr;
    std::map<Uint,Uint> zone_node_idx;
    for (Uint n=0; n<used_nodes.size(); ++n)
      zone_node_idx[ used_nodes[n] ] = n+1;

    const Uint nbCellsInType  = elements.size();


    // print zone header,
    // one zone per element type per cpu
    // therefore the title is dependent on those parameters
    file << "ZONE "
         << "  T=\"" << elements.uri().path() << "\""
         << ", N=" << used_nodes.size()
         << ", E=" << elements.size()
         << ", DATAPACKING=BLOCK"
         << ", ZONETYPE=" << zone_type(etype);
    if (cell_centered_var_ids.size() && options().option("cell_centred").value<bool>())
    {
      file << ",VARLOCATION=(["<<cell_centered_var_ids[0];
      for (Uint i=1; i<cell_centered_var_ids.size(); ++i)
        file << ","<<cell_centered_var_ids[i];
      file << "]=CELLCENTERED)";
    }
    file << "\n\n";


         //    fout << ", VARLOCATION=( [" << init_id << "]=CELLCENTERED )" ;
         //   else
         //    fout << ", VARLOCATION=( [" << init_id << "-" << end_id << "]=CELLCENTERED )" ;
         // }

    file.setf(std::ios::scientific,std::ios::floatfield);
    file.precision(12);

    // loop over coordinates
    const common::Table<Real>& coordinates = mesh.geometry_fields().coordinates();
    for (Uint d = 0; d < dimension; ++d)
    {
      file << "\n### variable x" << d << "\n\n"; // var name in comment
      boost_foreach(Uint n, used_nodes.array())
      {
        file << coordinates[n][d] << " ";
        CF3_BREAK_LINE(file,n);
      }
      file << "\n";
    }
    file << "\n";


    boost_foreach(Handle<Field> field_ptr, m_fields)
    {
      Field& field = *field_ptr;
      Uint var_idx(0);
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        Field::VarType var_type = field.var_length(iVar);
        std::string var_name = field.var_name(iVar);
        file << "\n### variable " << var_name << "\n\n"; // var name in comment

        for (Uint i=0; i<static_cast<Uint>(var_type); ++i)
        {
          if (field.basis() == SpaceFields::Basis::POINT_BASED)
          {
            if ( &field.field_group() == &mesh.geometry_fields() )
            {
              boost_foreach(Uint n, used_nodes.array())
              {
                file << field[n][var_idx] << " ";
                CF3_BREAK_LINE(file,n);
              }
              file << "\n";
            }
            else
            {
              if (field.field_group().defined_for_entities(elements.handle<Entities>()) )
              {
                Space& field_space = field.space(elements);
                RealVector field_data (field_space.nb_states());

                std::vector<Real> nodal_data(used_nodes.size(),0.);

                RealMatrix interpolation(elements.geometry_space().nb_states(),field_space.nb_states());
                const RealMatrix& geometry_local_coords = elements.geometry_space().shape_function().local_coordinates();
                const ShapeFunction& sf = field_space.shape_function();
                for (Uint g=0; g<interpolation.rows(); ++g)
                {
                  interpolation.row(g) = sf.value(geometry_local_coords.row(g));
                }

                for (Uint e=0; e<elements.size(); ++e)
                {
                  Connectivity::ConstRow field_index = field_space.indexes_for_element(e);

                  /// set field data
                  for (Uint iState=0; iState<field_space.nb_states(); ++iState)
                  {
                    field_data[iState] = field[field_index[iState]][var_idx];
                  }

                  /// evaluate field shape function in P0 space
                  RealVector geometry_field_data = interpolation*field_data;

                  Connectivity::ConstRow geom_nodes = elements.get_nodes(e);
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

                for (Uint n=0; n<nodal_data.size(); ++n)
                {
                  file << nodal_data[n] << " ";
                  CF3_BREAK_LINE(file,n)
                }
                file << "\n";
              }
            }
          }
          else // element based
          {
            if (field.field_group().defined_for_entities(elements.handle<Entities>()))
            {
              Space& field_space = field.space(elements);
              RealVector field_data (field_space.nb_states());

              if (options().option("cell_centred").value<bool>())
              {
                boost::shared_ptr< ShapeFunction > P0_cell_centred = boost::dynamic_pointer_cast<ShapeFunction>(build_component("cf3.mesh.LagrangeP1."+to_str(elements.element_type().shape_name()),"tmp_shape_func"));

                for (Uint e=0; e<elements.size(); ++e)
                {
                  Connectivity::ConstRow field_index = field_space.indexes_for_element(e);
                  /// set field data
                  for (Uint iState=0; iState<field_space.nb_states(); ++iState)
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
                file << "\n";
              }
              else
              {
                std::vector<Real> nodal_data(used_nodes.size(),0.);
                std::vector<Uint> nodal_data_count(used_nodes.size(),0u);

                RealMatrix interpolation(elements.geometry_space().nb_states(),field_space.nb_states());
                const RealMatrix& geometry_local_coords = elements.geometry_space().shape_function().local_coordinates();
                const ShapeFunction& sf = field_space.shape_function();
                for (Uint g=0; g<interpolation.rows(); ++g)
                {
                  interpolation.row(g) = sf.value(geometry_local_coords.row(g));
                }

                for (Uint e=0; e<elements.size(); ++e)
                {
                  Connectivity::ConstRow field_index = field_space.indexes_for_element(e);

                  /// set field data
                  for (Uint iState=0; iState<field_space.nb_states(); ++iState)
                  {
                    field_data[iState] = field[field_index[iState]][var_idx];
                  }

                  /// evaluate field shape function in P0 space
                  RealVector geometry_field_data = interpolation*field_data;

                  Connectivity::ConstRow geom_nodes = elements.get_nodes(e);
                  cf3_assert(geometry_field_data.size()==geom_nodes.size());
                  /// Average nodal values
                  for (Uint g=0; g<geom_nodes.size(); ++g)
                  {
                    const Uint geom_node = geom_nodes[g];
                    const Uint node_idx = zone_node_idx[geom_node]-1;
                    cf3_assert(node_idx < nodal_data.size());
                    const Real accumulated_weight = nodal_data_count[node_idx]/(nodal_data_count[node_idx]+1.0);
                    const Real add_weight = 1.0/(nodal_data_count[node_idx]+1.0);
                    nodal_data[node_idx] = accumulated_weight*nodal_data[node_idx] + add_weight*geometry_field_data[g];
                    ++nodal_data_count[node_idx];
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
              if (options().option("cell_centred").value<bool>())
                file << elements.size() << "*" << 0.;
              else
                file << used_nodes.size() << "*" << 0.;
              file << "\n";
            }
          }
          var_idx++;
        }
      }
    }

    file << "\n### connectivity\n\n";
    // write connectivity
    boost_foreach( Connectivity::ConstRow e_nodes, elements.node_connectivity().array() )
    {
      boost_foreach ( Uint n, e_nodes)
      {
        file << zone_node_idx[n] << " ";
      }
      file << "\n";
    }
    file << "\n\n";

  }
}


std::string Writer::zone_type(const ElementType& etype) const
{
  if ( etype.shape() == GeoShape::LINE)     return "FELINESEG";
  if ( etype.shape() == GeoShape::TRIAG)    return "FETRIANGLE";
  if ( etype.shape() == GeoShape::QUAD)     return "FEQUADRILATERAL";
  if ( etype.shape() == GeoShape::TETRA)    return "FETETRAHEDRON";
  if ( etype.shape() == GeoShape::PYRAM)    return "FEBRICK";  // with coalesced nodes
  if ( etype.shape() == GeoShape::PRISM)    return "FEBRICK";  // with coalesced nodes
  if ( etype.shape() == GeoShape::HEXA)     return "FEBRICK";
  if ( etype.shape() == GeoShape::POINT)    return "FELINESEG"; // with coalesced nodes
  cf3_assert_desc("should not be here",false);
  return "INVALID";
}
////////////////////////////////////////////////////////////////////////////////

} // tecplot
} // mesh
} // cf3
