// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CHash_hpp
#define CF_Mesh_CHash_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CHash component class
/// This class serves as a component that that will hash
/// objects and return which partition or processor it belongs to
/// @author Willem Deconinck
class Mesh_API CHash : public Common::Component {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CHash> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CHash const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CHash ( const std::string& name );

  /// Virtual destructor
  virtual ~CHash() {}

  /// Get the class name
  static std::string type_name () { return "CHash"; }

  Uint part_of_obj(const Uint obj) const;

  Uint proc_of_part(const Uint part) const;

	Uint proc_of_obj(const Uint obj) const;

	Uint nb_objects_in_part(const Uint part) const;

	Uint nb_objects_in_proc(const Uint proc) const;

	Uint start_idx_in_part(const Uint part) const;

	Uint end_idx_in_part(const Uint part) const;

	Uint start_idx_in_proc(const Uint proc) const;

	Uint end_idx_in_proc(const Uint proc) const;

  bool owns(const Uint obj) const;

  Uint part_size() const;

private:

  Uint m_nb_obj;

  Uint m_base;

  Uint m_nb_parts;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CHash_hpp
