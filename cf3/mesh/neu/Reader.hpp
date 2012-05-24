// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_neu_Reader_hpp
#define cf3_mesh_neu_Reader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshReader.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/neu/LibNeu.hpp"
#include "mesh/neu/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  class Elements;
  class Region;
  class MergedParallelDistribution;
namespace neu {

//////////////////////////////////////////////////////////////////////////////

/// This class defines neutral mesh format reader
/// @author Willem Deconinck
class neu_API Reader : public MeshReader, public Shared
{
private: // typedefs

  typedef std::pair<Handle<Elements>,Uint> Region_TableIndex_pair;

public: // functions
  /// constructor
  Reader( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Reader"; }

  virtual std::string get_format() { return "neu"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void config_repartition();

  void read_headerData();

  void find_ghost_nodes();

  void read_coordinates();

  void read_connectivity();

  void read_groups();

  void read_boundaries();

  void get_file_positions();

  std::string element_type(const Uint neu_type, const Uint nb_nodes);

private: // data

  virtual void do_read_mesh_into(const common::URI& fp, Mesh& mesh);

  enum HashType { NODES=0, ELEMS=1 };
  Handle<MergedParallelDistribution> m_hash;

  // map< global index , pair< temporary table, index in temporary table > >
  std::map<Uint,Region_TableIndex_pair> m_global_to_tmp;

  boost::filesystem::fstream m_file;
  Handle<Mesh> m_mesh;
  Handle<Region> m_region;
  Handle< Region > m_tmp;

  std::set<Uint> m_ghost_nodes;
  std::map<Uint,Uint> m_node_to_coord_idx;

  Uint m_nodal_coordinates_position;
  Uint m_elements_cells_position;
  std::vector<Uint> m_element_group_positions;
  std::vector<Uint> m_boundary_condition_positions;

  struct HeaderData
  {
    // NUMNP    Total number of nodal points in the mesh
    // NELEM    Total number of elements in the mesh
    // NGPRS    Number of element groups
    // NBSETS   Number of boundary condition sets
    // NDFCD    Number of coordinate directions (2 or 3)
    // NDFVL    Number of velocity components (2 or 3)
    Uint NUMNP, NELEM, NGRPS, NBSETS, NDFCD, NDFVL;
    std::string mesh_name;
  } m_headerData;

  struct GroupData
  {
    // NGP      Element group number
    // NELGP    Number of elements in group
    // MTYP     Material type (0=Undefined, 1=Conjugate, 2=Fluid, 3=Porous, 4=Solid, 5=Deformable)
    // NFLAGS   Number of solver-dependent flags
    // ELMMAT   Identifying name of element group (or entity or zone)
    // ELEM     Vector of element indices
    Uint NGP, NELGP, MTYP, NFLAGS;
    std::string ELMMAT;
    std::vector<Uint> ELEM;
  };

  struct BCData
  {
    // NAME     Name of boundary-condition set
    // ITYPE    Data type (0 = node; 1 = element/cell)
    // NENTRY   Number of data records in boundary-condition set
    // NVALUES  Number of values for each data record
    // IBCODE1  (Optional) Boundary condition code 1
    std::string NAME;
    Uint ITYPE, NENTRY, NVALUES, IBCODE1;
  };

}; // end Reader

////////////////////////////////////////////////////////////////////////////////

} // neu
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_Reader_hpp
