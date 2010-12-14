// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_HeatConductionLinearSteady_hpp
#define CF_UFEM_HeatConductionLinearSteady_hpp

#include "Actions/Proto/BlockAccumulator.hpp"
#include "Actions/Proto/Terminals.hpp"

#include "Solver/CMethod.hpp"

#include "LibUFEM.hpp"

namespace CF {
namespace UFEM {

/// Wizard to set up steady linear heat conduction
class UFEM_API HeatConductionLinearSteady : public Solver::CMethod
{
public: // typedefs

  typedef boost::shared_ptr<HeatConductionLinearSteady> Ptr;
  typedef boost::shared_ptr<HeatConductionLinearSteady const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  HeatConductionLinearSteady ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "HeatConductionLinearSteady"; }

  // functions specific to the HeatConductionLinearSteady component
  /// Set up the equation
  void initialize();

  /// @name SIGNALS
  //@{

  /// Signal to run the model
  void run(Common::XmlNode& node);
  
  /// Signal to add Dirichlet boundary conditions
  void add_dirichlet_bc( Common::XmlNode& node );
  
  //@} END SIGNALS
private:
  // LSS variable needs to be persistent
  Actions::Proto::MeshTerm<2, Actions::Proto::LSS> m_blocks;
};

} // UFEM
} // CF


#endif // CF_UFEM_HeatConductionLinearSteady_hpp
