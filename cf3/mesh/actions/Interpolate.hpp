// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_Interpolate_hpp
#define cf3_mesh_actions_Interpolate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"
#include "mesh/Field.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Octtree;
  class Field;
  class Elements;

namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Interpolate Fields with matching support
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class mesh_actions_API Interpolate : public MeshTransformer
{
public: // typedefs

    typedef boost::shared_ptr<Interpolate> Ptr;
    typedef boost::shared_ptr<Interpolate const> ConstPtr;

public: // functions

  /// constructor
  Interpolate( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Interpolate"; }

  virtual void execute();

  /// Interpolate from a source field to a Table at coordinates given in another Table
  /// @param [in]  source       interpolate from this field
  /// @param [in]  coordinates  interpolate at these coordinates (rows are coordinates)
  /// @param [out] target       Table of interpolated values at the given coordinates
  /// @post target is resized: row-size from source, nb_rows from coordinates
  /// @note MPI communication is used if coordinates are not found on this rank. Other ranks
  ///       then interpolate and send result back
  void interpolate(const Field& source, const common::Table<Real>& coordinates, common::Table<Real>& target);

  void signal_interpolate ( common::SignalArgs& node);
  void signature_interpolate ( common::SignalArgs& node);

private:

  /// source field
  boost::weak_ptr<Field const> m_source;

  std::string m_source_space;

  /// target field
  boost::weak_ptr<Field> m_target;

  /// source octtree
  boost::shared_ptr<Octtree> m_octtree;

  void interpolate_coordinate(const RealVector& target_coord, const Elements& element_component, const Uint element_idx, Field::Row target_row);


}; // end Interpolate


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_Interpolate_hpp
