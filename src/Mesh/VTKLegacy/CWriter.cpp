// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <boost/assign/list_of.hpp>

#include "Common/BoostFilesystem.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/PE/Comm.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/VTKLegacy/CWriter.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/Field.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Mesh {
namespace VTKLegacy {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < VTKLegacy::CWriter, CMeshWriter, LibVTKLegacy> aVTKLegacyWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const std::string& name )
: CMeshWriter(name)
{

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".vtk");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_from_to(const CMesh& mesh, const URI& file_path)
{
  m_mesh = mesh.as_ptr<CMesh>().get();

  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path(file_path.path());
  if (PE::Comm::instance().size() > 1)
  {
    path = boost::filesystem::basename(path) + "_P" + to_str(PE::Comm::instance().rank()) + boost::filesystem::extension(path);
  }

  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }

  file
    << "# vtk DataFile Version 2.0\n"
    << "Exported by COOLFLuiD\n"
    << "ASCII\n"
    << "DATASET UNSTRUCTURED_GRID\n";

  const Field& coords = mesh.topology().geometry().coordinates();
  const Uint npoints = coords.size();
  const Uint dim = coords.row_size();

  // Output point coordinates
  file << "POINTS " << npoints << " double\n";
  for(Uint i = 0; i != npoints; ++i)
  {
    const Field::ConstRow row = coords[i];
    for(Uint j = 0; j != dim; ++j)
      file << " " << row[j];
    if(dim == 2) file << " " << 0.;
    file << "\n";
  }

  // map for element types
  std::map<GeoShape::Type,int> etype_map = boost::assign::map_list_of
    (GeoShape::TRIAG,5)
    (GeoShape::QUAD, 9)
    (GeoShape::TETRA, 10)
    (GeoShape::HEXA, 12);

  // Count number of elements, and the total number of connectivity nodes
  Uint nb_elems = 0;
  Uint nb_nodes = 0;
  boost_foreach(const CElements& elements, find_components_recursively<CElements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      nb_elems += elements.size();
      nb_nodes += elements.size() + elements.size() * elements.element_type().nb_nodes();
    }
  }

  // Output connectivity data
  file << "\nCELLS " << nb_elems << " " << nb_nodes << "\n";
  boost_foreach(const CElements& elements, find_components_recursively<CElements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const Uint n_elems = elements.size();
      const CConnectivity& conn_table = elements.node_connectivity();
      const Uint n_el_nodes = elements.element_type().nb_nodes();
      for(Uint i = 0; i != n_elems; ++i)
      {
        file << " " << n_el_nodes;
        const CConnectivity::ConstRow row = conn_table[i];
        for(Uint j = 0; j != n_el_nodes; ++j)
          file << " " << row[j];
        file << "\n";
      }
    }
  }

  // Output element types
  file << "\nCELL_TYPES " << nb_elems << "\n";
  boost_foreach(const CElements& elements, find_components_recursively<CElements>(m_mesh->topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const int vtk_e_type = etype_map[elements.element_type().shape()];
      const Uint n_elems = elements.size();
      for(Uint i = 0; i != n_elems; ++i)
        file << " " << vtk_e_type << "\n";
    }
  }

  // Output point fields TODO: support cell-centered data
  if(!m_fields.empty())
    file << "\nPOINT_DATA " << npoints << "\n";

  boost_foreach(boost::weak_ptr<Field> field_ptr, m_fields)
  {
    const Field& field = *field_ptr.lock();

    // must be point based
    if(field.basis() != FieldGroup::Basis::POINT_BASED)
      continue;

    // size must be correct
    if(field.size() != npoints)
      continue;

    for(Uint var_idx = 0; var_idx != field.nb_vars(); ++var_idx)
    {
      const std::string var_name = field.var_name(var_idx);
      const Uint var_begin = field.var_index(var_name);
      if(field.var_length(var_idx) == Field::SCALAR)
      {
        file << "SCALARS " << var_name << " double\nLOOKUP_TABLE default\n";
        for(Uint i = 0; i != npoints; ++i)
          file << " " << field[i][var_begin] << "\n";
      }
      else if(static_cast<Uint>(field.var_length(var_idx)) == dim)
      {
        file << "VECTORS " << var_name << " double\n";
        const Uint var_end = var_begin+dim;
        for(Uint i = 0; i != npoints; ++i)
        {
          const Field::ConstRow row = field[i];
          for(Uint j = var_begin; j != var_end; ++j)
            file << " " << field[i][j];
          if(dim == 2) file << " " << 0.;
          file << "\n";
        }
      }
    }
  }

  file.close();

}

////////////////////////////////////////////////////////////////////////////////

} // VTKLegacy
} // Mesh
} // CF
