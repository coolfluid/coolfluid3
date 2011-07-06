// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_HeatConductionSteady_hpp
#define CF_UFEM_HeatConductionSteady_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "SteadyModel.hpp"

namespace CF {

namespace UFEM {

/// Special case of steady FEM problems: only one LSS solve is needed to get the solution of the problem
/// Useful for i.e. linear heat conduction
class UFEM_API HeatConductionSteady : public SteadyModel
{
public: // typedefs

  typedef boost::shared_ptr<HeatConductionSteady> Ptr;
  typedef boost::shared_ptr<HeatConductionSteady const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  HeatConductionSteady ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "HeatConductionSteady"; }
};

} // UFEM
} // CF


#endif // CF_UFEM_HeatConductionSteady_hpp
