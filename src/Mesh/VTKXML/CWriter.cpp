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
#include "Common/MPI/PE.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"

#include "Common/XML/FileOperations.hpp"
#include "Common/XML/XmlDoc.hpp"
#include "Common/XML/XmlNode.hpp"

#include "Mesh/VTKXML/CWriter.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/Field.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Common::XML;

namespace CF {
namespace Mesh {
namespace VTKXML {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < VTKXML::CWriter, CMeshWriter, LibVTKXML> aVTKXMLWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const std::string& name )
: CMeshWriter(name)
{

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".pvtu");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_from_to(const CMesh& mesh, const URI& file_path)
{
  XmlDoc vtkfile("1.0", "ISO-8859-1");

  // Root node
  XmlNode root = vtkfile.add_node("VTKFile");
  root.set_attribute("type", "UnstructuredGrid");
  root.set_attribute("version", "0.1");
  root.set_attribute("byte_order", "LittleEndian");
  root.set_attribute("compressor", "vtkZLibDataCompressor");
  
  XmlNode unstructured_grid = root.add_node("UnstructuredGrid");
  
  const Field& coords = mesh.topology().geometry().coordinates();
  const Uint npoints = coords.size();
  const Uint dim = coords.row_size();
  
  // map for element types
  std::map<GeoShape::Type,int> etype_map = boost::assign::map_list_of
    (GeoShape::TRIAG,5)
    (GeoShape::QUAD, 9)
    (GeoShape::TETRA, 10)
    (GeoShape::HEXA, 12);
  
  // Count number of elements
  Uint nb_elems = 0;
  boost_foreach(const CElements& elements, find_components_recursively<CElements>(mesh.topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      nb_elems += elements.size();
    }
  }
  
  XmlNode piece = unstructured_grid.add_node("Piece");
  piece.set_attribute("NumberOfPoints", to_str(npoints));
  piece.set_attribute("NumberOfCells", to_str(nb_elems));
  
  // Points output
  std::stringstream coords_stream;
  coords_stream << "\n";
  for(Uint i = 0; i != npoints; ++i)
  {
    const Field::ConstRow row = coords[i];
    for(Uint j = 0; j != dim; ++j)
      coords_stream << " " << row[j];
    if(dim == 2) coords_stream << " " << 0.;
    coords_stream << "\n";
  }
  
  XmlNode points_data = piece.add_node("Points").add_node("DataArray", coords_stream.str());
  points_data.set_attribute("type", "Float64");
  points_data.set_attribute("NumberOfComponents", "3");
  points_data.set_attribute("format", "ascii");
  coords_stream.str(std::string()); // free mem
  
  
  XmlNode cells = piece.add_node("Cells");
  
  // Cell connectivity
  std::stringstream conn_stream, offset_stream, type_stream;
  conn_stream << "\n"; offset_stream << "\n"; type_stream << "\n";
  Uint offset = 0;
  boost_foreach(const CElements& elements, find_components_recursively<CElements>(mesh.topology()) )
  {
    if(elements.element_type().dimensionality() == dim && elements.element_type().order() == 1 && etype_map.count(elements.element_type().shape()))
    {
      const Uint n_elems = elements.size();
      const CConnectivity& conn_table = elements.node_connectivity();
      const Uint n_el_nodes = elements.element_type().nb_nodes();
      const int vtk_e_type = etype_map[elements.element_type().shape()];
      for(Uint i = 0; i != n_elems; ++i)
      {
        const CConnectivity::ConstRow row = conn_table[i];
        for(Uint j = 0; j != n_el_nodes; ++j)
          conn_stream << " " << row[j];
        conn_stream << "\n";
        offset += n_el_nodes;
        offset_stream << " " << offset;
        type_stream << " " << vtk_e_type;
      }
    }
  }
  
  XmlNode connectivity = cells.add_node("DataArray", conn_stream.str());
  connectivity.set_attribute("type", "Int32");
  connectivity.set_attribute("Name", "connectivity");
  connectivity.set_attribute("format", "ascii");
  
  XmlNode offsets = cells.add_node("DataArray", offset_stream.str());
  offsets.set_attribute("type", "Int32");
  offsets.set_attribute("Name", "offsets");
  offsets.set_attribute("format", "ascii");
  
  XmlNode types = cells.add_node("DataArray", type_stream.str());
  types.set_attribute("type", "UInt8");
  types.set_attribute("Name", "types");
  types.set_attribute("format", "ascii");
  
  XmlNode cell_data = piece.add_node("CellData");
  XmlNode point_data = piece.add_node("PointData");
  
  boost_foreach(boost::weak_ptr<Field> field_ptr, m_fields)
  {
    const Field& field = *field_ptr.lock();

    // point-based field
    if(!(field.basis() == FieldGroup::Basis::POINT_BASED || field.basis() == CF::Mesh::FieldGroup::Basis::ELEMENT_BASED))
      continue;

    for(Uint var_idx = 0; var_idx != field.nb_vars(); ++var_idx)
    {
      std::stringstream field_stream;
      field_stream << "\n";
      
      const std::string var_name = field.var_name(var_idx);
      const Uint var_begin = field.var_index(var_name);
      const Uint field_size = field.size();
      const Uint var_size = field.var_length(var_idx);
      for(Uint i = 0; i != field_size; ++i)
        for(Uint j = 0; j != var_size; ++j)
          field_stream << " " << field[i][j] << "\n";
      
      
      XmlNode data_array = FieldGroup::Basis::POINT_BASED == field.basis()
        ? point_data.add_node("DataArray", field_stream.str()) 
        : cell_data.add_node("DataArray", field_stream.str());
       
      data_array.set_attribute("type", "Float64");
      data_array.set_attribute("NumberOfComponents", to_str(var_size));
      data_array.set_attribute("Name", var_name);
      data_array.set_attribute("format", "ascii");
    }
  }

  to_file(vtkfile, file_path.path());
}

////////////////////////////////////////////////////////////////////////////////

} // VTKXML
} // Mesh
} // CF
