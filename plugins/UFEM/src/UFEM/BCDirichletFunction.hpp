// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCDirichletFunction_hpp
#define cf3_UFEM_BCDirichletFunction_hpp

#include "solver/actions/Proto/DirichletBC.hpp"
#include "ParsedFunctionExpression.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Boundary condition to hold the value of a field at a value given by another (or the same) field
class UFEM_API BCDirichletFunction : public ParsedFunctionExpression
{
public:

  /// Contructor
  /// @param name of the component
  BCDirichletFunction ( const std::string& name );
  
  virtual ~BCDirichletFunction();

  /// Get the class name
  static std::string type_name () { return "BCDirichletFunction"; }
  
  virtual void execute();
private:
  cf3::solver::actions::Proto::DirichletBC m_dirichlet;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BCDirichletFunction_hpp
