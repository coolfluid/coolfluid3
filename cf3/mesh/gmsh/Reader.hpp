// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Gmsh_Reader_hpp
#define cf3_mesh_Gmsh_Reader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <boost/tuple/tuple.hpp>

#include "mesh/MeshReader.hpp"

#include "mesh/gmsh/LibGmsh.hpp"
#include "mesh/gmsh/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

class Elements;
class Region;
class MergedParallelDistribution;
class Dictionary;

class Mesh;

namespace gmsh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines gmsh mesh format reader
/// @author Willem Deconinck
/// @author Martin Vymazal
class gmsh_API Reader : public MeshReader, public Shared
{
public: // typedefs

  
  

private: // typedefs

public: // functions
  /// constructor
  Reader( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Reader"; }

  virtual std::string get_format() { return "Gmsh"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void get_file_positions();

  Handle<Region> create_region(std::string const& relative_path);

  void find_used_nodes();

//  void find_ghost_nodes();

  void read_coordinates();

  void read_connectivity();

  void read_element_data();

  void read_node_data();

private: // data

  virtual void do_read_mesh_into(const common::URI& fp, Mesh& mesh);

  enum HashType { NODES=0, ELEMS=1 };
  Handle<MergedParallelDistribution> m_hash;

  // map< gmsh index , pair< elements, index in elements > >
  std::map<Uint, boost::tuple<Handle<Elements>,Uint> > m_elem_idx_gmsh_to_cf;
  std::map<Uint, Uint> m_node_idx_gmsh_to_cf;

  boost::filesystem::fstream m_file;
  Handle<Mesh> m_mesh;
  Handle<Region> m_region;

  std::string m_file_basename;

  struct RegionData
  {
    Uint dim;
    Uint index;
    std::string name;
    Handle<Region> region;
    std::set<Uint> element_types;
  };

  Uint m_nb_regions; // This corresponds to the number of physical groups in
                     // gmsh terminology
  Uint m_mesh_dimension;


  std::vector<RegionData> m_region_list;

  std::set<Uint> m_ghost_nodes;
  //std::set<Uint> m_ghost_elems;
  std::set<Uint> m_used_nodes;
  
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
    Real time;
    Uint time_step;
    std::vector<Uint> var_types;
    Uint nb_entries;
    std::vector<Uint> file_data_positions;
  };

  void fix_negative_volumes(Mesh& mesh);

  void read_variable_header(std::map<std::string,Field>& fields);

  std::string var_type_gmsh_to_cf(const Uint& var_type_gmsh);

  Uint IO_rank;
}; // end Reader

////////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Gmsh_Reader_hpp
