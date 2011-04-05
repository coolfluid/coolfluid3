// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshElements_hpp
#define CF_Mesh_CMeshElements_hpp

#include "Mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  class CMesh;

////////////////////////////////////////////////////////////////////////////////

/// This class allows to access data spread over multiple components
/// with a continuous index
/// @pre the data components must be of the same type and must have
///      a member function "Uint size() const" defined.
class Mesh_API CMeshElements : public CUnifiedData
{
public: //typedefs

  typedef boost::shared_ptr<CMeshElements> Ptr;
  typedef boost::shared_ptr<CMeshElements const> ConstPtr;
  
public: // functions

  /// Contructor
  /// @param name of the component
  CMeshElements ( const std::string& name );

  /// Virtual destructor
  virtual ~CMeshElements() {}

  /// Get the class name
  static std::string type_name () { return "CMeshElements"; }

  void update();
  
}; // CMeshElements

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshElements_hpp
