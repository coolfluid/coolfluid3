// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_BoundaryConditions_hpp
#define CF_UFEM_BoundaryConditions_hpp

#include "Common/CActionDirector.hpp"
#include "Common/OptionURI.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/CSolver.hpp"

#include "Solver/Actions/Proto/BlockAccumulator.hpp"
#include "Solver/Actions/Proto/CProtoActionDirector.hpp"
#include "Solver/Actions/Proto/DirichletBC.hpp"

#include "LibUFEM.hpp"

namespace CF {

namespace UFEM {

/// BoundaryConditions for UFEM problems
class UFEM_API BoundaryConditions : public Solver::Actions::Proto::CProtoActionDirector
{
public: // typedefs

  typedef boost::shared_ptr<BoundaryConditions> Ptr;
  typedef boost::shared_ptr<BoundaryConditions const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  BoundaryConditions ( const std::string& name );
  
  virtual ~BoundaryConditions();

  /// Get the class name
  static std::string type_name () { return "BoundaryConditions"; }
  
  /// Add a constant dirichlet BC
  /// @param region_name Name of the boundary region. Must be unique in the problem region
  /// @param variable_name Name of the variable for which to set the BC
  /// @param default_value Default value
  CAction& add_constant_bc(const std::string& region_name, const std::string& variable_name, const boost::any default_value);
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // CF


#endif // CF_UFEM_BoundaryConditions_hpp
