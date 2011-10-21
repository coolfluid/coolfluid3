// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Zoltan_CPartitioner_hpp
#define cf3_mesh_Zoltan_CPartitioner_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshPartitioner.hpp"
#include "mesh/Zoltan/LibZoltan.hpp"

namespace cf3 {
namespace common { template <typename KEY, typename DATA> class Map;}
namespace mesh {
namespace Zoltan {

////////////////////////////////////////////////////////////////////////////////

/// MeshPartitioner component class
/// This class serves as a component that that will partition the mesh
/// @author Willem Deconinck
class Zoltan_API CPartitioner : public MeshPartitioner {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CPartitioner> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CPartitioner const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CPartitioner ( const std::string& name );

  /// Virtual destructor
  virtual ~CPartitioner();

  /// Get the class name
  static std::string type_name () { return "CPartitioner"; }

  /// Partitioning functions

  virtual void build_graph() { /* Does nothing, as partition_graph() will use call-back functions to construct the graph */ }

  virtual void partition_graph();

private: // functions

  /// returns the handle to zoltan
  /// @pre must be called after the PE is initialized
  ZoltanHandle& zoltan_handle();

  void set_partitioning_params();

  // query function
  static int  query_nb_of_objects(void *data, int *ierr);

  static void query_list_of_objects(void *data, int sizeGID, int sizeLID,
                                   ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                   int wgt_dim, float *obj_wgts, int *ierr);

  static void query_nb_connected_objects(void *data, int sizeGID, int sizeLID, int num_obj,
                                         ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                         int *numEdges, int *ierr);

  static void query_list_of_connected_objects(void *data, int sizeGID, int sizeLID, int num_obj,
                                              ZOLTAN_ID_PTR globalID, ZOLTAN_ID_PTR localID,
                                              int *num_edges,
                                              ZOLTAN_ID_PTR nborGID, int *nborProc,
                                              int wgt_dim, float *ewgts, int *ierr);




private: // data

  bool m_partitioned;
  int changes;
  int numGidEntries;
  int numLidEntries;
  int numImport;
  ZOLTAN_ID_PTR importGlobalIds;
  ZOLTAN_ID_PTR importLocalIds;
  int *importProcs;
  int *importToPart;
  int numExport;
  ZOLTAN_ID_PTR exportGlobalIds;
  ZOLTAN_ID_PTR exportLocalIds;
  int *exportProcs;
  int *exportToPart;

  boost::shared_ptr<ZoltanHandle> m_zz;
  Real m_zoltan_version;
};

////////////////////////////////////////////////////////////////////////////////

} // Zoltan
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Zoltan_CPartitioner_hpp
