// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Gmsh_CReader_hpp
#define CF_Mesh_Gmsh_CReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <boost/tuple/tuple.hpp>

#include "Mesh/CMeshReader.hpp"

#include "Mesh/Gmsh/LibGmsh.hpp"
#include "Mesh/Gmsh/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

class CElements;
class CRegion;
class CMixedHash;
class CNodes;
class CMesh;

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

public: // functions
  /// constructor
  CReader( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "CReader"; }

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void get_file_positions();

  boost::shared_ptr<CRegion> create_region(std::string const& relative_path);

  void find_ghost_nodes();

  void read_coordinates();

  void read_connectivity();

  void read_element_data();

  void read_node_data();

  virtual void read_from_to(const Common::URI& fp, CMesh& mesh);

private: // data

  enum HashType { NODES=0, ELEMS=1 };
  boost::shared_ptr<CMixedHash> m_hash;

  // map< gmsh index , pair< elements, index in elements > >
  std::map<Uint, boost::tuple<boost::shared_ptr<CElements>,Uint> > m_elem_idx_gmsh_to_cf;
  std::map<Uint, Uint> m_node_idx_gmsh_to_cf;

  boost::filesystem::fstream m_file;
  boost::shared_ptr<CMesh> m_mesh;
  boost::shared_ptr<CRegion> m_region;
  boost::shared_ptr<CRegion> m_tmp;

  boost::shared_ptr<CNodes> m_nodes;
  std::string m_file_basename;

  struct RegionData
  {
    Uint dim;
    Uint index;
    std::string name;
    boost::shared_ptr<CRegion> region;
    std::set<Uint> element_types;
  };

  Uint m_nb_regions; // This corresponds to the number of physical groups in
                     // Gmsh terminology
  Uint m_mesh_dimension;


  std::vector<RegionData> m_region_list;

  std::set<Uint> m_ghost_nodes;
  //std::set<Uint> m_ghost_elems;
  std::set<Uint> m_nodes_to_read;

  std::vector<std::set<Uint> > m_node_to_glb_elements;

  //Markers for important places in the file to be read
  Uint m_region_names_position;
  Uint m_coordinates_position;
  Uint m_elements_position;
  std::vector<Uint> m_element_data_positions;
  std::vector<Uint> m_node_data_positions;
  std::vector<Uint> m_element_node_data_positions;


  std::vector<std::vector<Uint> > m_nb_gmsh_elem_in_region;
  Uint m_total_nb_elements;
  Uint m_total_nb_nodes;

  struct Field
  {
    std::string name;
    std::vector<std::string> var_names;
    std::string topology;
    std::string basis;
    Real time;
    Uint time_step;
    std::vector<Uint> var_types;
    Uint nb_entries;
    std::vector<Uint> file_data_positions;
  };

  void read_variable_header(std::map<std::string,Field>& fields);

  std::string var_type_gmsh_to_cf(const Uint& var_type_gmsh);

}; // end CReader

////////////////////////////////////////////////////////////////////////////////

} // Gmsh
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Gmsh_CReader_hpp
