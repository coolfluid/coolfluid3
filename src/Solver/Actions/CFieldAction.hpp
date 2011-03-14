// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CFieldAction_hpp
#define CF_Solver_Actions_CFieldAction_hpp

#include "Common/CAction.hpp"
#include "Math/MatrixTypes.hpp"

#include "LibActions.hpp"

namespace CF {
  namespace Mesh { class CMesh; }
namespace Solver {
namespace Actions {

/// Abstract base for components that can act on a field
struct Solver_Actions_API CFieldAction : public Common::CAction
{
  typedef std::vector<std::string> StringsT;
  typedef std::vector<Uint> SizesT;
  typedef boost::shared_ptr< CFieldAction > Ptr;
  typedef boost::shared_ptr< CFieldAction const> ConstPtr;
  
  CFieldAction(const std::string& name) : Common::CAction(name)
  {
  }
  
  static std::string type_name() { return "CFieldAction"; }
  
  virtual ~CFieldAction() {}
  
  /// List the variable names
  virtual StringsT variable_names() const = 0;
  
  /// List the field name for each variable
  virtual StringsT field_names() const = 0;
  
  /// Number of Real numbers needed to store each variable
  virtual SizesT variable_sizes() const = 0;
  
  virtual Uint nb_dofs() const = 0;
  
  /// Create the fields needed for the action to operate
  virtual void create_fields() = 0;
  
  /// Update the fields from a global solution vector
  virtual void update_fields(const RealVector& solution, Mesh::CMesh& solution_mesh) = 0;
};

} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_CFieldAction_hpp
