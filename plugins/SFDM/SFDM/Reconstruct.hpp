// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_Reconstruct_hpp
#define CF_SFDM_Reconstruct_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/LibSFDM.hpp"
#include "Math/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

class ShapeFunction;

//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that reconstructs a state from one
/// shapefunction to another.
/// Example: States defined in solution points need to be reconstructed
///          in Flux points.
/// @author Willem Deconinck
class SFDM_API Reconstruct : public common::Component
{
public: // typedefs

    typedef boost::shared_ptr<Reconstruct> Ptr;
    typedef boost::shared_ptr<Reconstruct const> ConstPtr;

public: // functions

  /// constructor
  Reconstruct( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Reconstruct"; }

  /// Reconstructed values from the states from shapefunction "from"
  /// to the locations from shapefunction "to"
  /// @param from_states  States in locations from shapefunction "from". dimensions (nb_states x state_size)
  RealMatrix value(const RealMatrix& from_states) const;

  /// Reconstructed gradient from the states from shapefunction "from"
  /// to the locations from shapefunction "to"
  /// @param from_states  States in locations from shapefunction "from". dimensions (nb_states x state_size)
  /// @param orientation  Direction to which the derivative is taken (KSI / ETA / ZTA)
  RealMatrix gradient(const RealMatrix& from_states, const CoordRef orientation) const;

private:

  void configure_from_to();

  RealMatrix m_value_reconstruction_matrix;

  std::vector<RealMatrix> m_gradient_reconstruction_matrix;

  boost::shared_ptr<ShapeFunction> m_from;
  boost::shared_ptr<ShapeFunction> m_to;

}; // end Reconstruct


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_Reconstruct_hpp
