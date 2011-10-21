// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CSimpleMeshGenerator_hpp
#define cf3_mesh_CSimpleMeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "CMeshGenerator.hpp"
#include "common/PE/Comm.hpp"
namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// CSimpleMeshGenerator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CSimpleMeshGenerator : public CMeshGenerator {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CSimpleMeshGenerator> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CSimpleMeshGenerator const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSimpleMeshGenerator ( const std::string& name );

  /// Virtual destructor
  virtual ~CSimpleMeshGenerator();

  /// Get the class name
  static std::string type_name () { return "CSimpleMeshGenerator"; }

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

#endif // cf3_mesh_CSimpleMeshGenerator_hpp
