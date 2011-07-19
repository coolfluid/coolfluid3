// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Action_hpp
#define CF_Solver_Action_hpp

#include "Common/CAction.hpp"
#include "Common/FindComponents.hpp"
#include "Solver/LibSolver.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh { class CRegion; class CMesh; }
namespace Physics { class PhysModel; }
namespace Solver {

  class CTime;

/////////////////////////////////////////////////////////////////////////////////////

class Solver_API Action : public Common::CAction
{
public: // typedefs

  /// provider
  typedef boost::shared_ptr< Action > Ptr;
  typedef boost::shared_ptr< Action const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Action ( const std::string& name );

  void config_regions();

  /// Virtual destructor
  virtual ~Action();

  /// Get the class name
  static std::string type_name () { return "Action"; }

 public:

  Mesh::CMesh& mesh();

  Physics::PhysModel& physical_model();

  Solver::CTime& time();

  Common::ComponentIteratorRange<Mesh::CRegion> regions();

protected:

  /// mesh where this action data resides
  boost::weak_ptr< Mesh::CMesh > m_mesh;
  /// physical model used by this action
  boost::weak_ptr< Physics::PhysModel > m_physical_model;
  /// time used by this action
  boost::weak_ptr< Solver::CTime > m_time;

  /// regions of the mesh to loop over
  std::vector< boost::shared_ptr< Mesh::CRegion > > m_loop_regions;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Action_hpp
