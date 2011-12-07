// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RiemannSolvers_LibRiemannSolvers_hpp
#define cf3_RiemannSolvers_LibRiemannSolvers_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro RiemannSolvers_API
/// @note build system defines COOLFLUID_RIEMANNSOLVERS_EXPORTS when compiling RiemannSolvers files
#ifdef COOLFLUID_RIEMANNSOLVERS_EXPORTS
#   define RiemannSolvers_API      CF3_EXPORT_API
#   define RiemannSolvers_TEMPLATE
#else
#   define RiemannSolvers_API      CF3_IMPORT_API
#   define RiemannSolvers_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
/// @brief Riemann Solver classes
///
/// A Riemann solver is a numerical method used to solve a Riemann problem. 
/// They are heavily used in computational fluid dynamics and computational
/// magnetohydrodynamics.
///
/// Riemann Solvers are used to compute the flux going through a discontinuous
/// boundary. 
/// @f[ H = Riemann( U_L , U_R ) @f]
/// with @f$ U_L @f$ and @f$ U_R @f$ the state on the left, and the state on the right
/// respectively.
/// @author Willem Deconinck
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the RiemannSolvers library
/// @author Willem Deconinck
class RiemannSolvers_API LibRiemannSolvers :
    public common::Library
{
public:

  
  

  /// Constructor
  LibRiemannSolvers ( const std::string& name) : common::Library(name) { }

  virtual ~LibRiemannSolvers() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.RiemannSolvers"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "RiemannSolvers"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements components to construct a Riemann Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibRiemannSolvers"; }

}; // end LibRiemannSolvers

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_RiemannSolversLibRiemannSolvers_hpp
