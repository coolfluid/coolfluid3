// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ptscotch_Partitioner_hpp
#define cf3_mesh_ptscotch_Partitioner_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshPartitioner.hpp"
#include "mesh/ptscotch/LibPTScotch.hpp"

namespace cf3 {
namespace mesh {
namespace ptscotch {

////////////////////////////////////////////////////////////////////////////////

/// MeshPartitioner component class
/// This class serves as a component that that will partition the mesh
/// @author Willem Deconinck
class mesh_ptscotch_API Partitioner : public MeshPartitioner {

public: // typedefs

  /// type of pointer to Component
  
  /// type of pointer to constant Component
  

public: // functions

  /// Contructor
  /// @param name of the component
  Partitioner ( const std::string& name );

  /// Virtual destructor
  virtual ~Partitioner();

  /// Get the class name
  static std::string type_name () { return "Partitioner"; }

  /// Partitioning functions
  
  virtual void build_graph();
  
  virtual void partition_graph();
  
private: // functions

  
  /// common values accessed by all tests goes here
  SCOTCH_Dgraph graph;

  SCOTCH_Num baseval;                 // first index of an array starts with 0 for c++
  SCOTCH_Num vertglbnbr;              // number of vertices in the total mesh
  SCOTCH_Num edgeglbnbr;              // number of connections in the total mesh
  SCOTCH_Num procglbnbr;              // number of processors

  SCOTCH_Num vertlocnbr;              // number of vertices on this processor
  SCOTCH_Num vertgstnbr;              // number of vertices on this processor, including ghost vertices
  SCOTCH_Num edgelocnbr;              // number of connections to other vertices starting from each local vertex
  
  SCOTCH_Num vertlocmax;
  SCOTCH_Num edgelocsiz;
  std::vector<SCOTCH_Num> vertloctab;
  std::vector<SCOTCH_Num> edgeloctab;
  std::vector<SCOTCH_Num> edgegsttab;
  std::vector<SCOTCH_Num> partloctab;
  std::vector<SCOTCH_Num> proccnttab;// number of vertices per processor
  std::vector<SCOTCH_Num> procvrttab;// start_idx of the vertex for each processor + one extra index greater than vertglbnbr
};

////////////////////////////////////////////////////////////////////////////////

} // ptscotch
} // mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ptscotch_Partitioner_hpp
