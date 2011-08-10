// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/BoostFilesystem.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/Tecplot/CWriter.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace Mesh {
namespace Tecplot {

#define CF_BREAK_LINE(f,x) { if( x+1 % 10) { f << "\n"; } }

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Tecplot::CWriter, CMeshWriter, LibTecplot> aTecplotWriter_Builder;

//////////////////////////////////////////////////////////////////////////////

CWriter::CWriter( const std::string& name )
: CMeshWriter(name)
{

}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CWriter::get_extensions()
{
  std::vector<std::string> extensions;
  extensions.push_back(".plt");
  return extensions;
}

/////////////////////////////////////////////////////////////////////////////

void CWriter::write_from_to(const CMesh& mesh, const URI& file_path)
{

  m_mesh = mesh.as_ptr<CMesh>().get();

  // if the file is present open it
  boost::filesystem::fstream file;
  boost::filesystem::path path(file_path.path());
  if (mpi::PE::instance().size() > 1)
  {
    path = boost::filesystem::basename(path) + "_P" + to_str(mpi::PE::instance().rank()) + boost::filesystem::extension(path);
  }
//  CFLog(VERBOSE, "Opening file " <<  path.string() << "\n");
  file.open(path,std::ios_base::out);
  if (!file) // didn't open so throw exception
  {
     throw boost::filesystem::filesystem_error( path.string() + " failed to open",
                                                boost::system::error_code() );
  }


  write_file(file);


  file.close();

}
/////////////////////////////////////////////////////////////////////////////

void CWriter::write_file(std::fstream& file)
{
  file << "TITLE      = COOLFluiD Mesh Data" << "\n";
  file << "VARIABLES  = ";

  Uint dimension = m_mesh->nodes().coordinates().row_size();
  // write the coordinate variable names
  for (Uint i = 0; i < dimension ; ++i)
  {
    file << " \"x" << i << "\" ";
  }

  std::vector<Uint> cell_centered_var_ids;
  Uint zone_var_id(dimension);
  boost_foreach(boost::weak_ptr<CField> field_ptr, m_fields)
  {
    CField& field = *field_ptr.lock();
    for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
    {
      CField::VarType var_type = field.var_type(iVar);
      std::string var_name = field.var_name(iVar);

      if ( static_cast<Uint>(var_type) > 1)
      {
        for (Uint i=0; i<static_cast<Uint>(var_type); ++i)
        {
          file << " \"" << var_name << "["<<i<<"]\"";
          ++zone_var_id;
          if (field.basis() != CField::Basis::POINT_BASED)
            cell_centered_var_ids.push_back(zone_var_id);
        }
      }
      else
      {
        file << " \"" << var_name <<"\"";
        ++zone_var_id;
        if (field.basis() != CField::Basis::POINT_BASED)
          cell_centered_var_ids.push_back(zone_var_id);
      }
    }
  }
  file << "\n";


  // loop over the element types
  // and create a zone in the tecplot file for each element type
  std::map<Component::ConstPtr,Uint> zone_id;
  Uint zone_idx=0;
  boost_foreach (const CElements& elements, find_components_recursively<CElements>(m_mesh->topology()) )
  {
    const ElementType& etype = elements.element_type();
    if (etype.shape() == GeoShape::POINT)
      continue;
    // Tecplot doesn't handle zones with 0 elements
    // which can happen in parallel, so skip them
    if (elements.size() == 0)
      continue;

    zone_id[elements.self()] = zone_idx++;

    CList<Uint>& used_nodes = CEntities::used_nodes(*elements.as_non_const(),true); // VERY DIRTY HACK to remove constness!!!
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
    if (cell_centered_var_ids.size())
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
    const CTable<Real>& coordinates = m_mesh->nodes().coordinates();
    for (Uint d = 0; d < dimension; ++d)
    {
      file << "\n### variable x" << d << "\n\n"; // var name in comment
      boost_foreach(Uint n, used_nodes.array())
      {
        file << coordinates[n][d] << " ";
        CF_BREAK_LINE(file,n);
      }
      file << "\n";
    }
    file << "\n";


    boost_foreach(boost::weak_ptr<CField> field_ptr, m_fields)
    {
      CField& field = *field_ptr.lock();
      Uint var_idx(0);
      for (Uint iVar=0; iVar<field.nb_vars(); ++iVar)
      {
        CField::VarType var_type = field.var_type(iVar);
        std::string var_name = field.var_name(iVar);
        file << "\n### variable " << var_name << "\n\n"; // var name in comment

        for (Uint i=0; i<static_cast<Uint>(var_type); ++i)
        {
          if (field.basis() == CField::Basis::POINT_BASED)
          {
            boost_foreach(Uint n, used_nodes.array())
            {
              file << field[n][var_idx] << " ";
              CF_BREAK_LINE(file,n);
            }
            file << "\n";
          }
          else // element based
          {
            CMultiStateFieldView field_view("field_view");
            field_view.set_field(field);
            if (field_view.set_elements(elements))
            {
              RealVector field_data (field_view.space().nb_states());

              for (Uint e=0; e<elements.size(); ++e)
              {
                /// set field data
                CMultiStateFieldView::View data_view = field_view[e];
                for (Uint iState=0; iState<field_view.space().nb_states(); ++iState)
                {
                  field_data[iState] = data_view[iState][var_idx];
                }

                /// get cell-centred local coordinates
                RealVector local_coords = elements.space("P0").shape_function().local_coordinates().row(0);

                /// evaluate field shape function in P0 space
                Real cell_centred_data = field_view.space().shape_function().value(local_coords)*field_data;

                /// Write cell centred value
                file << cell_centred_data << " ";
                CF_BREAK_LINE(file,e);
              }
              file << "\n";
            }
            else
            {
              // field not defined for this zone, so write zeros
              file << elements.size() << "*" << 0.;
              file << "\n";
            }
          }
          var_idx++;
        }
      }
    }

    file << "\n### connectivity\n\n";
    // write connectivity
    boost_foreach( CConnectivity::ConstRow e_nodes, elements.node_connectivity().array() )
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


std::string CWriter::zone_type(const ElementType& etype) const
{
  if ( etype.shape() == GeoShape::LINE)     return "FELINESEG";
  if ( etype.shape() == GeoShape::TRIAG)    return "FETRIANGLE";
  if ( etype.shape() == GeoShape::QUAD)     return "FEQUADRILATERAL";
  if ( etype.shape() == GeoShape::TETRA)    return "FETETRAHEDRON";
  if ( etype.shape() == GeoShape::PYRAM)    return "FEBRICK";  // with coalesced nodes
  if ( etype.shape() == GeoShape::PRISM)    return "FEBRICK";  // with coalesced nodes
  if ( etype.shape() == GeoShape::HEXA)     return "FEBRICK";
  if ( etype.shape() == GeoShape::POINT)    return "FELINESEG"; // with coalesced nodes
  cf_assert_desc("should not be here",false);
  return "INVALID";
}
////////////////////////////////////////////////////////////////////////////////

} // Tecplot
} // Mesh
} // CF
