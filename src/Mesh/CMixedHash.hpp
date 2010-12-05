// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMixedHash_hpp
#define CF_Mesh_CMixedHash_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/CMap.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

  class CHash;
  
////////////////////////////////////////////////////////////////////////////////

/// CMixedHash component class
/// This class serves as a component that that will hash
/// objects and return which partition or processor it belongs to
/// @author Willem Deconinck
class Mesh_API CMixedHash : public Common::Component {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CMixedHash> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CMixedHash const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMixedHash ( const std::string& name );

  /// Virtual destructor
  virtual ~CMixedHash() {}

  /// Get the class name
  static std::string type_name () { return "CMixedHash"; }

  Uint part_of_obj(const Uint obj) const;

  Uint proc_of_part(const Uint part) const;
  
  Uint proc_of_obj(const Uint obj) const;
	
	Uint nb_objects_in_part(const Uint part) const;
	
	Uint nb_objects_in_proc(const Uint proc) const;
	
	Uint start_idx_in_part(const Uint part) const;
	
	Uint end_idx_in_part(const Uint part) const;
	
	Uint start_idx_in_proc(const Uint proc) const;

  bool owns(const Uint obj) const;

  Uint part_size() const;
  
  boost::shared_ptr<CHash const> subhash(const Uint i) const
  {
    return m_subhash[i];
  }
  
  Uint subhash_of_obj(const Uint obj) const;
  
private:

  void config_nb_obj();
  
  void config_nb_parts();
  
  std::vector<Uint> m_nb_obj;

  Uint m_base;

  Uint m_nb_parts;
  
  std::vector<boost::shared_ptr<CHash> > m_subhash;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMixedHash_hpp
