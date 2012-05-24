// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ElementFinder_hpp
#define cf3_mesh_ElementFinder_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "mesh/LibMesh.hpp"
#include "math/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class SpaceElem;
  class Dictionary;

//////////////////////////////////////////////////////////////////////////////

/// @brief Base class for finding an element given a coordinate
class Mesh_API ElementFinder : public common::Component
{
public:

  /// @brief type name
  static std::string type_name() {return "ElementFinder"; }

  /// @brief Constructor
  ElementFinder(const std::string& name);

  /// @brief Find which element contains a given coordinate
  /// @param [in]  target_coord   The coordinate used to find the element
  /// @param [out] element        The found element
  /// @return if element was found
  virtual bool find_element(const RealVector& target_coord, SpaceElem& element) = 0;

protected:
  Handle<Dictionary> m_dict;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementFinder_hpp
