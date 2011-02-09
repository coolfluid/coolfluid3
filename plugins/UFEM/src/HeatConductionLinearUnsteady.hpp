// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_HeatConductionLinearUnsteady_hpp
#define CF_UFEM_HeatConductionLinearUnsteady_hpp

#include "LibUFEM.hpp"
#include "LinearSystemUnsteady.hpp"

namespace CF {
namespace UFEM {

/// Wizard to set up steady linear heat conduction
class UFEM_API HeatConductionLinearUnsteady : public LinearSystemUnsteady
{
public: // typedefs

  typedef boost::shared_ptr<HeatConductionLinearUnsteady> Ptr;
  typedef boost::shared_ptr<HeatConductionLinearUnsteady const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  HeatConductionLinearUnsteady ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "HeatConductionLinearUnsteady"; }

protected: // Linear system interface implementation
  virtual Solver::Actions::CFieldAction::Ptr build_equation();
};

} // UFEM
} // CF


#endif // CF_UFEM_HeatConductionLinearUnsteady_hpp
