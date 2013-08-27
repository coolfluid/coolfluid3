// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/PropertyList.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/Table.hpp"
#include "common/List.hpp"
#include "common/DynTable.hpp"

#include "common/PE/debug.hpp"

#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ConnectivityData.hpp"
#include "mesh/MergedParallelDistribution.hpp"
#include "mesh/ParallelDistribution.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/Cells.hpp"
#include "smurf/smurf.h"

#include "mesh/smurf/Reader.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace smurf {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

cf3::common::ComponentBuilder < smurf::Reader, MeshReader, LibSmurf> aSmurfReader_Builder;

//////////////////////////////////////////////////////////////////////////////

Reader::Reader( const std::string& name )
: MeshReader(name),
  Shared()
{

  // options
//  options().add("part", PE::Comm::instance().rank() )
//      .description("Number of the part of the mesh to read. (e.g. rank of processor)")
//      .pretty_name("Part");

//  options().add("nb_parts", PE::Comm::instance().size() )
//      .description("Total number of parts. (e.g. number of processors)")
//      .pretty_name("nb_parts");

  options().add("read_fields", true)
      .description("Read the data from the mesh")
      .pretty_name("Read Fields")
      .mark_basic();

  // properties

  properties()["brief"] = std::string("Smurf file reader component");

//  std::string desc;
//  desc += "This component can read in parallel.\n";
//  desc += "It can also read multiple files in serial, combining them in one large mesh.\n";
//  desc += "Available coolfluid-element types are:\n";
//  boost_foreach(const std::string& supported_type, m_supported_types)
//  desc += "  - " + supported_type + "\n";
//  properties()["description"] = desc;

  IO_rank = 0;
}

//////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Reader::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".smurf");
  return extensions;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::do_read_mesh_into(const URI& file, Mesh& mesh)
{

  // if the file is present open it
  boost::filesystem::path fp (file.path());
  if(!boost::filesystem::exists(fp))
    throw boost::filesystem::filesystem_error( fp.string() + " does not exist", boost::system::error_code() );

  m_file_basename = boost::filesystem::basename(fp);

  // set the internal mesh pointer
  m_mesh = Handle<Mesh>(mesh.handle<Component>());

  // Create a region component inside the mesh with a generic mesh name
  // NOTE: since gmsh contains several 'physical entities' in one mesh, we create one region per physical entity
  m_region = Handle<Region>(m_mesh->topology().handle<Component>());

  CFinfo <<  "smurf: opening file " <<  fp.string() << CFendl;
  SmURF::MeshReader mreader(fp.string());

  std::string title_;
  std::vector<std::string> vn_;

  // headers section
  mreader.readMainHeader(title_,vn_);

  CFdebug << "smurf: reading mesh \"" << title_ << "\"" << CFendl;

  m_mesh_dimension = options().value<Uint>("dimension");
  std::vector< SmURF::TecZone > zheaders = mreader.readZoneHeaders();

  // data section
  Uint i = 0;
  boost_foreach( SmURF::TecZone& zone, zheaders)
  {
    std::vector< std::vector< unsigned > > ve;
    std::vector< std::vector< double > > dump;

    // ASSUME ALL VARIABLE DATA IS IN ZONE 1 (VARSHARELISTS!!)
    mreader.readZoneData(zone,ve,!i?m_vv:dump);
    if (dump.size())
      throw NotSupported(FromHere(), "Sorry! Only binary tecplot files with varsharelists are supported (i.e. all node data should be attributed to the first zone)");

    std::string cf_elem_name;
    // correct connectivity numbering
    if (ve.size() && zone.type==SmURF::FELINESEG) {

      // collapse line segments to points
      if (ve[0][0]==ve[0][1]) {
        cf_elem_name = "cf3.mesh.LagrangeP0.Point1D";
        for (unsigned c=0; c<ve.size(); ++c) {
          ve[c].resize(1);
        }
      }
    }
    else if (ve.size() && zone.type==SmURF::FEBRICK) {

      if (ve[0][2]==ve[0][3] && ve[0][6]==ve[0][7]) { //*2

        // collapse an hexahedron to a prism
        cf_elem_name = "cf3.mesh.LagrangeP1.Prism3D";
        for (unsigned c=0; c<ve.size(); ++c) {
          const std::vector< unsigned >  ent = ve[c];  // element nodes, original (tecplot)
                std::vector< unsigned >& eng = ve[c];  // ...,           modified (gambit)
          eng.resize(6);
          eng[0] = ent[0];
          eng[1] = ent[1];
          eng[2] = ent[2];
          eng[3] = ent[4];
          eng[4] = ent[5];
          eng[5] = ent[6];
        }

      }
      else if ((ve[0][4]==ve[0][5]) && (ve[0][4]==ve[0][6]) && (ve[0][4]==ve[0][7])) { //*3

        // collapse an hexahedron to a pyramid
        cf_elem_name = "cf3.mesh.LagrangeP1.Pyramid3D";
        for (unsigned c=0; c<ve.size(); ++c) {
          const std::vector< unsigned >  ent = ve[c];  // element nodes, original (tecplot)
                std::vector< unsigned >& eng = ve[c];  // ...,           modified (gambit)
          eng.resize(5);
          eng[0] = ent[0];
          eng[1] = ent[1];
          eng[2] = ent[3];
          eng[3] = ent[2];
          eng[4] = ent[4];
        }

      }
      else { //*1

        // correct node ordering for bricks
        for (unsigned c=0; c<ve.size(); ++c) {
          std::swap(ve[c][2],ve[c][3]);
          std::swap(ve[c][6],ve[c][7]);
        }
      }
    }

    cf_elem_name = cf_elem_name==""?Shared::tp_name_to_cf_name(m_tp_elem_dim[zone.type], zone.type):cf_elem_name;

    // Read variables
    CFdebug << "smurf: reading zone \"" << zone.title << "\"" << CFendl
            << "       time : " << zone.time << CFendl
            << "       nodes: " << zone.i << CFendl
            << "       elems: " << zone.j << CFendl
            << "       etype: " << cf_elem_name << CFendl;

    m_mesh_dimension = std::max(Shared::m_tp_elem_dim[zone.type],m_mesh_dimension);

    Handle<Region> region = create_region(zone.title);

    Dictionary& nodes = m_mesh->geometry_fields();

    boost::shared_ptr< ElementType > allocated_type = build_component_abstract_type<ElementType>(cf_elem_name,"tmp");
    boost::shared_ptr< Entities > elements;
    if (allocated_type->dimensionality() == allocated_type->dimension()-1)
      elements = build_component_abstract_type<Entities>("cf3.mesh.Faces","elements_"+allocated_type->derived_type_name());
    else if(allocated_type->dimensionality() == allocated_type->dimension())
      elements = build_component_abstract_type<Entities>("cf3.mesh.Cells","elements_"+allocated_type->derived_type_name());
    else
      elements = build_component_abstract_type<Entities>("cf3.mesh.Elements","elements_"+allocated_type->derived_type_name());
    region->add_component(elements);
    elements->initialize(cf_elem_name,nodes);

    Connectivity& elem_table = Handle<Elements>(elements)->geometry_space().connectivity();
    elem_table.set_row_size(Shared::m_nodes_in_tp_elem[zone.type]);
    elem_table.resize(ve.size());
    elements->rank()   .resize(ve.size()); // something to do with parallel?
    elements->glb_idx().resize(ve.size()); // something to do with parallel?
    for (int i=0; i<ve.size(); ++i)
      for (int j=0; j<ve[i].size(); ++j)
        elem_table[i][j] = ve[i][j];
    ++i;
  }

  m_total_nb_nodes = m_vv[0].size();

  CFdebug << "smurf: nb_nodes : " << m_total_nb_nodes << CFendl
          << "       dimension: " << Shared::dim_name[m_mesh_dimension] << CFendl;

  cf3_assert(is_not_null(m_mesh));
  m_mesh->initialize_nodes(m_used_nodes.size(), m_mesh_dimension);


  // coordinates
  Dictionary& nodes = m_mesh->geometry_fields();
  nodes.resize(m_vv[0].size());
  for (Uint dim=0; dim<m_mesh_dimension; ++dim)
    for (Uint j=0; j<m_vv[0].size(); ++j)
      nodes.coordinates()[j][dim] = m_vv[dim][j];

  // other fields (one field per variable)
  for (Uint i=m_mesh_dimension; i<m_vv.size(); ++i)
  {
    CFdebug << "       variable : " << vn_[i] <<CFendl;
    mesh::Field& field = nodes.create_field(vn_[i]);
    for (Uint j=0; j<m_vv[0].size(); ++j)
      field[j][0] = m_vv[i][j];
  }

  fix_negative_volumes(*m_mesh);

  mesh.raise_mesh_loaded(); // something to do with parallel?
}

////////////////////////////////////////////////////////////////////////////////

Handle< Region > Reader::create_region(std::string const& relative_path)
{
  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  boost::char_separator<char> sep("/");
  Tokenizer tokens(relative_path, sep);

  Handle< Region > region = m_region;
  for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    std::string name = *tok_iter;
    Handle< Component > new_region = region->get_child(name);
    if (is_null(new_region))  region->create_component<Region>(name);
    region = Handle<Region>(region->get_child(name));
  }
  return region;
}

//////////////////////////////////////////////////////////////////////////////

void Reader::fix_negative_volumes(Mesh& mesh)
{
  /// @note this is now only implemented for LagrangeP2.Quad2D elements!!! others are ignored
  boost_foreach(Cells& elements, find_components_recursively<Cells>(mesh.topology()))
  {
    if (elements.element_type().derived_type_name() == "cf3.mesh.LagrangeP2.Quad2D")
    {
      Real jacobian_determinant=0;
      Uint nb_nodes_per_elem = elements.element_type().nb_nodes();
      std::vector<Uint> tmp_nodes(nb_nodes_per_elem);
      for (Uint e=0; e<elements.size(); ++e)
      {
        jacobian_determinant = elements.element_type().jacobian_determinant(elements.geometry_space().shape_function().local_coordinates().row(0),elements.geometry_space().get_coordinates(e));
        if (jacobian_determinant < 0)
        {
          // reverse the connectivity nodes order
          for (Uint n=0;n<nb_nodes_per_elem; ++n)
            tmp_nodes[n] = elements.geometry_space().connectivity()[e][n];
          if (elements.element_type().derived_type_name() == "cf3.mesh.LagrangeP2.Quad2D")
          {
            elements.geometry_space().connectivity()[e][0] = tmp_nodes[0];
            elements.geometry_space().connectivity()[e][1] = tmp_nodes[3];
            elements.geometry_space().connectivity()[e][2] = tmp_nodes[2];
            elements.geometry_space().connectivity()[e][3] = tmp_nodes[1];
            elements.geometry_space().connectivity()[e][4] = tmp_nodes[7];
            elements.geometry_space().connectivity()[e][5] = tmp_nodes[6];
            elements.geometry_space().connectivity()[e][6] = tmp_nodes[5];
            elements.geometry_space().connectivity()[e][7] = tmp_nodes[4];
            elements.geometry_space().connectivity()[e][8] = tmp_nodes[8];
          }
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // smurf
} // mesh
} // cf3
