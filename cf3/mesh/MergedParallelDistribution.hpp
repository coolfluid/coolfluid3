// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MergedParallelDistribution_hpp
#define cf3_mesh_MergedParallelDistribution_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "common/Map.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class ParallelDistribution;

////////////////////////////////////////////////////////////////////////////////

/// MergedParallelDistribution component class
/// This class serves as a component that that will hash
/// objects and return which partition or processor it belongs to
/// @author Willem Deconinck
class Mesh_API MergedParallelDistribution : public common::Component {

public: // typedefs

  /// type of pointer to Component
  
  /// type of pointer to constant Component
  

public: // functions

  /// Contructor
  /// @param name of the component
  MergedParallelDistribution ( const std::string& name );

  /// Virtual destructor
  virtual ~MergedParallelDistribution() {}

  /// Get the class name
  static std::string type_name () { return "MergedParallelDistribution"; }

  Uint part_of_obj(const Uint obj) const;

  Uint proc_of_part(const Uint part) const;

  Uint proc_of_obj(const Uint obj) const;

  Uint nb_objects_in_part(const Uint part) const;

  Uint nb_objects_in_proc(const Uint proc) const;

  Uint start_idx_in_part(const Uint part) const;

  Uint end_idx_in_part(const Uint part) const;

  Uint start_idx_in_proc(const Uint proc) const;

  /// @deprecated function, use rank_owns() instead
  bool owns(const Uint obj) const { return rank_owns(obj); }

  bool rank_owns(const Uint obj) const;

  bool part_owns(const Uint part, const Uint obj) const;

  Uint part_size() const;

  const ParallelDistribution& subhash(const Uint i) const
  {
    return *m_subhash[i];
  }

  Uint subhash_of_obj(const Uint obj) const;

private:

  void config_nb_obj();

  void config_nb_parts();

  std::vector<Uint> m_nb_obj;

  Uint m_base;

  Uint m_nb_parts;

  std::vector<Handle<ParallelDistribution> > m_subhash;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MergedParallelDistribution_hpp
