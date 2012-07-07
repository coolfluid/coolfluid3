// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ElementFinderOcttree_hpp
#define cf3_mesh_ElementFinderOcttree_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/ElementFinder.hpp"
#include "mesh/Entities.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Octtree;
  
/// @brief Find elements using an octtree
class Mesh_API ElementFinderOcttree : public ElementFinder
{
public:

  /// @brief type name
  static std::string type_name() {return "ElementFinderOcttree"; }

  /// @brief Constructor
  ElementFinderOcttree(const std::string& name);

  virtual bool find_element(const RealVector& target_coord, SpaceElem& element);

private:

  void configure_octtree();

private:

  Handle<Octtree> m_octtree;
  Entity m_tmp;
  bool m_closest;

  std::vector<Uint> m_octtree_idx;

  std::vector<Entity> m_elements_pool;

  RealMatrix m_coordinates;


};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementFinderOcttree_hpp
