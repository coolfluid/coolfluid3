// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Convection_hpp
#define cf3_SFDM_Convection_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/Term.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RiemannSolvers { class RiemannSolver; }
namespace physics        { class Variables; }
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

    typedef boost::shared_ptr<Convection> Ptr;
    typedef boost::shared_ptr<Convection const> ConstPtr;

public: // functions

  /// constructor
  Convection( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "Convection"; }

  virtual void execute();

private:

  void compute_one_cell_at_a_time();
  void compute_cell_interior_flux_points_contribution();
  void compute_inner_face_flux_points_contribution();


  void trigger_physical_model();
  void build_riemann_solver();
  boost::shared_ptr<RiemannSolvers::RiemannSolver> m_riemann_solver;
  boost::weak_ptr<physics::Variables> m_solution_vars;

}; // end Convection


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_Convection_hpp
