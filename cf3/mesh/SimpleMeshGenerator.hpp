// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_SimpleMeshGenerator_hpp
#define cf3_mesh_SimpleMeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "MeshGenerator.hpp"
#include "common/PE/Comm.hpp"
namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// SimpleMeshGenerator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API SimpleMeshGenerator : public MeshGenerator {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<SimpleMeshGenerator> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<SimpleMeshGenerator const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SimpleMeshGenerator ( const std::string& name );

  /// Virtual destructor
  virtual ~SimpleMeshGenerator();

  /// Get the class name
  static std::string type_name () { return "SimpleMeshGenerator"; }

  virtual void execute();

private:

  /// Create a line
  void create_line();

  /// Create a rectangle
  void create_rectangle();

protected: // data

  std::vector<Uint> m_nb_cells;
  std::vector<Real> m_lengths;
  std::vector<Real> m_offsets;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_SimpleMeshGenerator_hpp
