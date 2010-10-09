// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CDiscretization_hpp
#define CF_Solver_CDiscretization_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CMethod.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API CDiscretization : public Solver::CMethod {

public: // typedefs

  /// type of this class contruction provider
  typedef Common::ConcreteProvider < CDiscretization,1 > PROVIDER;
  /// type of pointer to Component
  typedef boost::shared_ptr<CDiscretization> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CDiscretization const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CDiscretization ( const CName& name );

  /// Virtual destructor
  virtual ~CDiscretization();

  /// Get the class name
  static std::string type_name () { return "CDiscretization"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  // functions specific to the CDiscretization component

  virtual void do_whatever() = 0;
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CDiscretization_hpp
