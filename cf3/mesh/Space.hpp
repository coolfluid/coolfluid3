// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Space_hpp
#define cf3_mesh_Space_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Table_fwd.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class Connectivity;
  class ShapeFunction;
  class Dictionary;
  class Entities;

////////////////////////////////////////////////////////////////////////////////

/// @brief Space component class
///
/// A Space component uniquely relates to 1 Entities component.\n
/// The concept of "space" is here introduced as an invisible mesh parallel
/// to the original mesh. It has exactly the same elements as the original mesh,
/// but every element is defined by a different shape function.\n
/// A default space that is always created is the "geometry" space, defined by
/// the mesh (from mesh-readers / mesh-generators).\n
/// Space is a concept that allows to create fields in the same mesh or parts of the
/// mesh, with different shape functions than prescribed by the mesh.
/// This is useful for e.g. high-order discretization methods, without having to
/// generate a high-order mesh.
///
/// A class Dictionary is responsible for managing multiple Space components.
/// Fields are created in the dictionary. More than one field can be created in he same
/// dictionary, ensuring they have the same space and other common definitions.
/// A connectivity table which is held inside this Space component refers to entries
/// in the dictionary.
///
/// Example: @n
/// The coordinates of mesh element vertices is e.g. a field in the dictionary "geometry",
/// and the connectivity table of the space "geometry" refers to the
/// vertices connected to the mesh-elements.
///
/// Notes:
/// - Newly created spaces always have a coordinates field in their dictionary.
///
/// @author Willem Deconinck

class Mesh_API Space : public common::Component {

public: // functions

  /// @brief Get the class name
  static std::string type_name () { return "Space"; }

  /// @brief Contructor
  /// @param name of the component
  Space ( const std::string& name );

  /// @brief Initialize the Space
  ///
  /// Initialize function, must be called before any other function
  /// - Set private handles
  /// - Register this space in the dictionary  ( dict.add_space() )
  /// - Create a connectivity table
  /// @param [in]  support  Entities this space is parallel to
  /// @param [in]  dict     Dictionary this space belongs to
  Space& initialize(Entities& support, Dictionary& dict);

  /// @brief Virtual destructor
  virtual ~Space();

  /// @brief The number of elements defined in this space.
  ///
  /// This function delegates connectivity().size()
  /// @return number of elements
  Uint size() const;

  /// @brief Dictionary this space belongs to.
  ///
  /// This space's connectivity table refers to entries of the dictionary.
  Dictionary& dict() const { return *m_dict; }

  /// Shape function defining this space
  /// @pre the shape function must be configured first
  /// @return shape function
  const ShapeFunction& shape_function() const;

  /// @brief Access the geometric support
  /// @return a reference to the entities
  /// @throws SetupError if not set.
  Entities& support() const { return *m_support; }

  /// @brief connectivity table to dictionary entries
  /// @return connectivity table
  /// @pre Node connectivity must have been created beforehand.
  ///      This is normally done by the dictionary automatically,
  ///      except for the geometry space, which is filled in by
  ///      a mesh-reader or a mesh-generator.
  Connectivity& connectivity() { return *m_connectivity; }

  /// @brief connectivity table to dictionary entries
  /// @return connectivity table
  /// @pre Node connectivity must have been created beforehand.
  ///      This is normally done by the dictionary automatically,
  ///      except for the geometry space, which is filled in by
  ///      a mesh-reader or a mesh-generator.
  const Connectivity& connectivity() const { return *m_connectivity; }

  /// @brief Compute element coordinates
  ///
  /// Compute coordinates of every node belonging to element with given index
  /// This is computationally expensive, as it interpolates from the geometry-space,
  /// using the geometry-space's shape-function.
  /// @param [in] elem_idx element index
  /// @return element coordinates (nb_nodes x dimension)
  RealMatrix compute_coordinates(const Uint elem_idx) const;

  /// @brief Lookup element coordinates
  ///
  /// Coordinates accessed from the dictionary's coordinates-field,
  /// and copied into a RealMatrix
  /// @param [in] elem_idx element index
  /// @return element coordinates (nb_nodes x dimension)
  RealMatrix get_coordinates(const Uint elem_idx) const;

  /// @brief Lookup element coordinates
  ///
  /// Coordinates accessed from the dictionary's coordinates-field,
  /// and copied into a RealMatrix.
  /// This function is preferred over get_coordinates(), as no RealMatrix is allocated inside.
  /// @param [in]  elem_idx    element index
  /// @param [out] coordinates element coordinates (nb_nodes x dimension)
  void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  /// @brief Allocate element coordinates
  ///
  /// Allocate a properly sized coordinates matrix. Can be used in conjunction with
  /// put_coordinates()
  /// @param [out] coordinates  empty coordinates (nb_nodes x dimension)
  void allocate_coordinates(RealMatrix& coordinates) const;

private: // functions

  /// @brief Configuration option trigger for the shape function
  ///
  /// - Creates shape function
  /// - Resizes the connectivity table to the number of elements
  void configure_shape_function();

private: // data

  /// Shape function of this space
  Handle<ShapeFunction> m_shape_function;

  /// node_connectivity or state_connectivity for this space
  Handle<Connectivity> m_connectivity;

  /// Handle to the dictionary
  Handle<Dictionary> m_dict;

  /// Handle to the supporting Entities component
  Handle<Entities> m_support;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Space_hpp
