// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CList_hpp
#define CF_Mesh_CList_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/ListBase.hpp"
#include "Mesh/LibMesh.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table
/// The table has to be filled through a buffer.
/// Before using the table one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck Tiago Quintino
	
template <typename ValueT>
class Mesh_API CList : public Common::Component, public ListBase<ValueT> {

public:
  typedef boost::shared_ptr<CList> Ptr;
  typedef boost::shared_ptr<CList const> ConstPtr;
  
  /// Contructor
  /// @param name of the component
  CList ( const std::string& name ) :
		Component ( name ),
		ListBase<ValueT>()
	{
    BuildComponent<none>().build(this);
	}

  /// Get the class name
  static std::string type_name () { return "CList<"+class_name<ValueT>()+">"; }

};
	
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CList_hpp
