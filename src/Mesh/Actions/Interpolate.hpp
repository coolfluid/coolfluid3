// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Actions_Interpolate_hpp
#define CF_Mesh_Actions_Interpolate_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Actions/LibActions.hpp"
#include "Mesh/Field.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  class COcttree;
  class Field;
  class CElements;

namespace Actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Interpolate Fields with matching support
///
/// @post After this, the mesh is ready to be parallellized
/// @author Willem Deconinck
class Mesh_Actions_API Interpolate : public CMeshTransformer
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
  void interpolate(const Field& source, const CTable<Real>& coordinates, CTable<Real>& target);

  void signal_interpolate ( Common::SignalArgs& node);
  void signature_interpolate ( Common::SignalArgs& node);

private:

  /// source field
  boost::weak_ptr<Field const> m_source;

  std::string m_source_space;

  /// target field
  boost::weak_ptr<Field> m_target;

  /// source octtree
  boost::shared_ptr<COcttree> m_octtree;

  void interpolate_coordinate(const RealVector& target_coord, const CElements& element_component, const Uint element_idx, Field::Row target_row);


}; // end Interpolate


////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Actions_Interpolate_hpp
