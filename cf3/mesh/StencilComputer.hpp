// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_StencilComputer_hpp
#define cf3_mesh_StencilComputer_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "mesh/LibMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Dictionary;
  class SpaceElem;

//////////////////////////////////////////////////////////////////////////////

/// @brief Base class for stencil computers.
///
/// Given one element, a pool of elements is gathered
/// @author Willem Deconinck
class Mesh_API StencilComputer : public common::Component {

public: // functions  

  /// @brief Constructor
  StencilComputer( const std::string& name );
  
  /// @brief Gets the Class name
  static std::string type_name() { return "StencilComputer"; }

  /// @brief Compute the stencil around a given element
  /// @param [in]  element   The element to compute the stencil around
  /// @param [out] stencil   The computed stencil
  virtual void compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil) = 0;

protected: // data
  
  Handle<Dictionary> m_dict;      ///< The mesh used to compute the stencil
  Uint m_min_stencil_size;  ///< A minimum stencil size

}; // end StencilComputer

////////////////////////////////////////////////////////////////////////////////

/// @brief A StencilComputer returning one cell
///
/// This stencil computer actually doesn't compute, but returns
/// the given cell as stencil
/// @author Willem Deconinck
class Mesh_API StencilComputerOneCell : public StencilComputer {

public: // functions

  /// @brief Constructor
  StencilComputerOneCell( const std::string& name );

  /// @brief Gets the Class name
  static std::string type_name() { return "StencilComputerOneCell"; }

  virtual void compute_stencil(const SpaceElem& element, std::vector<SpaceElem>& stencil);

}; // end StencilComputerOneCell


////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_StencilComputer_hpp
