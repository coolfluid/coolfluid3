// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CPeriodicWriteMesh_hpp
#define CF_Solver_Actions_CPeriodicWriteMesh_hpp

#include "Common/CAction.hpp"

#include "Solver/Actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class CField; class CMesh; }
namespace Solver {
namespace Actions {

class Solver_Actions_API CPeriodicWriteMesh : public Common::CAction {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CPeriodicWriteMesh> Ptr;
  typedef boost::shared_ptr<CPeriodicWriteMesh const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CPeriodicWriteMesh ( const std::string& name );

  /// Virtual destructor
  virtual ~CPeriodicWriteMesh() {}

  /// Get the class name
  static std::string type_name () { return "CPeriodicWriteMesh"; }

  /// execute the action
  virtual void execute ();

private: // data

  boost::weak_ptr<Mesh::CMesh> m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

#endif // CF_Solver_Actions_CPeriodicWriteMesh_hpp
