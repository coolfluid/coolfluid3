// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CArray_hpp
#define CF_Mesh_CArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/ArrayBase.hpp"
#include "Mesh/MeshAPI.hpp"

#include "Mesh/BufferT.hpp"

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

/// Array component class
/// This class can store an array of reals
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CArray : public Common::Component, public ArrayBase<Real> {

public: // typedefs

  typedef boost::shared_ptr<CArray> Ptr;
  typedef boost::shared_ptr<CArray const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CArray ( const CName& name );

  /// Get the class name
  static std::string type_name () { return "CArray"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

std::ostream& operator<<(std::ostream& os, const CArray::ConstRow& row);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CArray_hpp
