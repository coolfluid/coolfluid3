// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CSimpleMeshGenerator_hpp
#define CF_Mesh_CSimpleMeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "CMeshGenerator.hpp"

namespace CF {
namespace Mesh {

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


protected: // functions

  void create_line(const Real x_len, const Uint x_segments);

  void create_rectangle(const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments);
  
protected: // data

  std::vector<Uint> m_nb_cells;
  std::vector<Real> m_lengths;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSimpleMeshGenerator_hpp
