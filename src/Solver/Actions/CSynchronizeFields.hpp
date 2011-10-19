// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CSynchronizeFields_hpp
#define cf3_Solver_Actions_CSynchronizeFields_hpp

#include "Solver/Action.hpp"

#include "Solver/Actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
	
namespace Mesh { class Field; }

namespace Solver {
namespace Actions {

class Solver_Actions_API CSynchronizeFields : public cf3::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CSynchronizeFields> Ptr;
  typedef boost::shared_ptr<CSynchronizeFields const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSynchronizeFields ( const std::string& name );

  /// Virtual destructor
  virtual ~CSynchronizeFields() {}

  /// Get the class name
  static std::string type_name () { return "CSynchronizeFields"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_fields();

private: // data

  std::vector< boost::weak_ptr<Mesh::Field> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

#endif // CF3_Solver_Actions_CSynchronizeFields_hpp
