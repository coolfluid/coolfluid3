// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Convection_hpp
#define cf3_SFDM_Convection_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"

#include "SFDM/Term.hpp"
#include "SFDM/ShapeFunction.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that creates a space in the mesh
/// with SFDM shape functions, and a configurable order of polynomial.
/// Default polynomial order = 0.
/// that returns information about the mesh
/// @author Willem Deconinck
class SFDM_API Convection : public Term
{
public: // typedefs
friend class Flyweight;
    typedef boost::shared_ptr<Convection> Ptr;
    typedef boost::shared_ptr<Convection const> ConstPtr;

public: // functions

  /// constructor
  Convection( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Convection"; }

  virtual void execute();

private: // functions

  void compute_interior_flx_pts_contribution();
  void compute_face_flx_pts_contribution();
  void apply_null_bc();

private: // configuration

  void allocate_fast_access_data();

private: // fast-access-data (for convenience no "m_" prefix)

  RealVector sol_in_flx_pt;
  RealVector flx_in_flx_pt;
  Real wave_speed_in_flx_pt;
  std::vector<Uint> face_at_side;
  std::vector<Uint> flx_pt_at_side;
  std::vector<RealVector> sol_at_side;

}; // end Convection


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_Convection_hpp
