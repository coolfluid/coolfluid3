// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Gmsh_CReader_hpp
#define CF_Mesh_Gmsh_CReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"

#include "Mesh/Gmsh/LibGmsh.hpp"
#include "Mesh/Gmsh/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

class CElements;
class CRegion;
class CMixedHash;

namespace Gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Gmsh mesh format reader
/// @author Willem Deconinck
/// @author Martin Vymazal
class Gmsh_API CReader : public CMeshReader, public Shared
{
public: // typedefs

  typedef boost::shared_ptr<CReader> Ptr;
  typedef boost::shared_ptr<CReader const> ConstPtr;

private: // typedefs

  typedef std::pair<boost::shared_ptr<CElements>,Uint> Region_TableIndex_pair;

public: // functions
  /// constructor
  CReader( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CReader"; }

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions
	
  void get_file_positions();

  void find_ghost_nodes();
	
  void read_coordinates();

  void read_connectivity();

  virtual void read_from_to(boost::filesystem::path& fp, const CMesh::Ptr& mesh);
	
private: // data

  enum HashType { NODES=0, ELEMS=1 };
  boost::shared_ptr<CMixedHash> m_hash;

  // map< global index , pair< temporary table, index in temporary table > >
  std::map<Uint,Region_TableIndex_pair> m_global_to_tmp;

  boost::filesystem::fstream m_file;
  CMesh::Ptr m_mesh;
  CRegion::Ptr m_region;
  CRegion::Ptr m_tmp;

  CNodes::Ptr m_nodes;
  std::string m_file_basename;

  struct RegionData 
  {
    Uint dim;
    Uint index;
    std::string name;
  };
	
  Uint m_nb_regions; // This corresponds to the number of physical groups in
                     // Gmsh terminology
  Uint m_mesh_dimension;


  std::vector<RegionData> m_region_list;

	std::set<Uint> m_ghost_nodes;
  //std::set<Uint> m_ghost_elems;
	std::set<Uint> m_nodes_to_read;
  std::map<Uint,Uint> m_node_to_coord_idx;
	
  std::vector<std::set<Uint> > m_node_to_glb_elements;

  //Markers for important places in the file to be read
  Uint m_region_names_position;
  Uint m_coordinates_position;
  Uint m_elements_position;


  std::vector<std::vector<Uint> > m_nb_gmsh_elem_in_region;
  Uint m_total_nb_elements;
  Uint m_total_nb_nodes;

}; // end CReader

////////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_CReader_hpp
