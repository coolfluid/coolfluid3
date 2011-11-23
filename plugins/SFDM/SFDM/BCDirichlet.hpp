// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_BCDirichlet_hpp
#define cf3_SFDM_BCDirichlet_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/VectorialFunction.hpp"

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
class SFDM_API BCDirichlet : public Term
{
public: // typedefs

    typedef boost::shared_ptr<BCDirichlet> Ptr;
    typedef boost::shared_ptr<BCDirichlet const> ConstPtr;

public: // functions

  /// constructor
  BCDirichlet( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "BCDirichlet"; }

  virtual void execute();

private:

  void config_function();

  boost::weak_ptr<physics::Variables> m_input_vars;  ///< access to the input variables

  math::VectorialFunction  m_function;    ///< function parser for the math formula

  RealVector sol_in_flx_pt;
  RealVector flx_in_flx_pt;
  Real wave_speed_in_flx_pt;

}; // end BCDirichlet


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_BCDirichlet_hpp
