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

  Handle<Region> create_region(std::string const& relative_path);

private: // data

  virtual void do_read_mesh_into(const common::URI& fp, Mesh& mesh);

  // map< smurf index , pair< elements, index in elements > >
  std::map<Uint, std::pair<Handle<Elements>,Uint> > m_elem_idx_smurf_to_cf;
  std::map<Uint, Uint> m_node_idx_smurf_to_cf;

  boost::filesystem::fstream m_file;
  Handle<Mesh> m_mesh;
  Handle<Region> m_region;

  std::string m_file_basename;

  Uint m_nb_regions; // This corresponds to the number of physical groups in
                     // smurf terminology
  Uint m_mesh_dimension;

  std::vector< std::vector< double > > m_vv;

  std::set<Uint> m_ghost_nodes;
  //std::set<Uint> m_ghost_elems;
  std::set<Uint> m_used_nodes;

  std::vector<std::set<Uint> > m_node_to_glb_elements;

  std::vector<std::vector<Uint> > m_nb_smurf_elem_in_region;
  Uint m_total_nb_elements;
  Uint m_total_nb_nodes;

  void fix_negative_volumes(Mesh& mesh);

  Uint IO_rank;
}; // end Reader

////////////////////////////////////////////////////////////////////////////////

} // smurf
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Smurf_Reader_hpp
