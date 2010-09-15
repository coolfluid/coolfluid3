// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CTable_hpp
#define CF_Mesh_CTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/ArrayBase.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/BufferT.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table
/// The table has to be filled through a buffer.
/// Before using the table one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck Tiago Quintino
class Mesh_API CTable : public Common::Component, public ArrayBase<Uint> {

public:
  typedef boost::shared_ptr<CTable> Ptr;
  typedef boost::shared_ptr<CTable const> ConstPtr;
  
  /// Contructor
  /// @param name of the component
  CTable ( const CName& name );

  /// Get the class name
  static std::string type_name () { return "CTable"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}  
};

std::ostream& operator<<(std::ostream& os, const CTable::ConstRow& row);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTable_hpp
