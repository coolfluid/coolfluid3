// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshReader_hpp
#define cf3_mesh_MeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostFilesystem.hpp"

#include "common/ArrayBufferT.hpp"
#include "common/Action.hpp"
#include "common/Table.hpp"


#include "mesh/LibMesh.hpp"
#include "mesh/Mesh.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;
  class Region;
  class Cells;
  class Faces;
  class Elements;

////////////////////////////////////////////////////////////////////////////////

/// MeshReader component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API MeshReader : public common::Action {

public: // typedefs

  /// type of pointer to Component

  /// type of pointer to constant Component


public: // functions

  /// Contructor
  /// @param name of the component
  MeshReader ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshReader();

  /// Get the class name
  static std::string type_name () { return "MeshReader"; }

  /// @name SIGNALS
  //@{

  /// @note: This doesn't read anything from the xml node argument.
  ///        It just reads the config options
  void signal_read( common::SignalArgs& node  );

  void read_signature( common::SignalArgs & node );

  //@} END SIGNALS

  // --------- Direct access ---------

  /// @return the name of the file format
  virtual std::string get_format() = 0;

  /// @return the list of possible extensions of the file format
  virtual std::vector<std::string> get_extensions() = 0;

  /// Read a given file to a given mesh. This calls a concrete implementation given by do_read_mesh_into
  /// @param [in]     path  the file to read in
  /// @param [in,out] mesh  the mesh to write to
  void read_mesh_into(const common::URI& path, Mesh& mesh);

  virtual void execute();

protected: // functions

  /// Map type from string to a common::Table<Uint>::Buffer
  typedef std::map<std::string,boost::shared_ptr< common::ArrayBufferT<Uint > > > BufferMap;

  /// Create element regions for each given type inside a given region
  /// @param [in] parent_region   Region in which the elementregions will be made
  /// @param [in] nodes           Nodes location to use for storing the cell nodes
  /// @param [in] etypes          List of element type names that will be used
  /// @return a BufferMap with key an etype name and value a buffer for the region
  ///         with name of the etype
  std::map<std::string,Handle<Elements> > create_cells_in_region(Region& parent_region, Dictionary& nodes,
                                   const std::vector<std::string>& etypes);

  std::map<std::string,Handle<Elements> > create_faces_in_region(Region& parent_region, Dictionary& nodes,
                                   const std::vector<std::string>& etypes);

  std::map<std::string,boost::shared_ptr< common::Table<Uint>::Buffer > > create_connectivity_buffermap (std::map<std::string,Handle<Elements> >& elems_map);


  /// remove all regions with empty connectivity tables inside a given region
  /// @param [in] parent_region  Region in which the removal will take place
  void remove_empty_element_regions(Region& parent_region);

protected: // data

  Handle<Mesh> m_mesh;
  common::URI m_file_path;

private:
  /// this function implements the concrete mesh reading algorithm and is called by read_mesh_into
  /// @param [in]     path  the file to read in
  /// @param [in,out] mesh  the mesh to write to
  virtual void do_read_mesh_into(const common::URI& path, Mesh& mesh) = 0;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshReader_hpp
