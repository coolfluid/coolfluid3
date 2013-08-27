// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Smurf_Reader_hpp
#define cf3_mesh_Smurf_Reader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <set>
#include <boost/tuple/tuple.hpp>

#include "mesh/MeshReader.hpp"

#include "mesh/smurf/LibSmurf.hpp"
#include "mesh/smurf/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

class Elements;
class Region;
class MergedParallelDistribution;
class Dictionary;

class Mesh;

namespace smurf {

//////////////////////////////////////////////////////////////////////////////

/// This class defines smurf mesh format reader
/// @author Willem Deconinck
/// @author Martin Vymazal
class smurf_API Reader : public MeshReader, public Shared
{
public: // functions
  /// constructor
  Reader( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Reader"; }

  virtual std::string get_format() { return "Smurf"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void get_file_positions();

  Handle<Region> create_region(std::string const& relative_path);


private: // data

  virtual void do_read_mesh_into(const common::URI& fp, Mesh& mesh);

  enum HashType { NODES=0, ELEMS=1 };
  Handle<MergedParallelDistribution> m_hash;

  // map< smurf index , pair< elements, index in elements > >
  std::map<Uint, std::pair<Handle<Elements>,Uint> > m_elem_idx_smurf_to_cf;
  std::map<Uint, Uint> m_node_idx_smurf_to_cf;

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
                     // smurf terminology
  Uint m_mesh_dimension;


  std::vector<RegionData> m_region_list;

  std::vector<Handle<Region> > m_regions;
  std::vector< std::vector< double > > m_vv;

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


  std::vector<std::vector<Uint> > m_nb_smurf_elem_in_region;
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
    std::string description() const
    {
      std::stringstream ss;
      for (Uint var=0; var<var_names.size(); ++var)
      {
        ss << var_names[var] << "["<<var_type_smurf_to_cf(var_types[var])<<"]";
        if (var<var_names.size()-1)
          ss << ",";
      }
      return ss.str();
    }
    std::string var_type_smurf_to_cf(const Uint& var_type_smurf) const
    {
      switch (var_type_smurf)
      {
        case 1:
          return "scalar";
        case 3:
          return "vector";
        case 9:
          return "tensor";
        default:
          throw common::FileFormatError(FromHere(),"Smurf variable type should be either 1(scalar), 3(vector), 9(tensor).");
      }
      return "null";
    }
  };

  void fix_negative_volumes(Mesh& mesh);

  void read_variable_header(std::map<std::string,Field>& fields);

  Uint IO_rank;
}; // end Reader

////////////////////////////////////////////////////////////////////////////////

} // smurf
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Smurf_Reader_hpp
