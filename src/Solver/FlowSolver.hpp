// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_FlowSolver_hpp
#define CF_Solver_FlowSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CSolver.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {

namespace Common  { class CAction; }
namespace Physics { class PhysModel; }
namespace Solver  { class CTime; }

namespace Mesh
{
  class CRegion;
  class CMesh;
}

namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// @brief General FlowSolver class
///
/// This FlowSolver is generic and offers basic configuration for
/// - computation of boundary
/// - computation of interior domain
/// - time tracking component
/// - action to delegate solver
///
/// @author Willem Deconinck
class Solver_API FlowSolver : public Solver::CSolver {

public: // typedefs

  typedef boost::shared_ptr<FlowSolver> Ptr;
  typedef boost::shared_ptr<FlowSolver const> ConstPtr;

public: // functions

  class Solver_API Tags : public NonInstantiable<Tags>
  {
    public:
    static const char * physical_model() { return "physical_model"; }
    static const char * mesh()           { return "mesh"; }
    static const char * time()           { return "time"; }

    static const char * bc()             { return "bc"; }
    static const char * inner()          { return "inner"; }
    static const char * solve()          { return "solve"; }
    static const char * setup()          { return "setup"; }

    static const char * solution()       { return "solution"; }
    static const char * residual()       { return "residual"; }
    static const char * wave_speed()     { return "wave_speed"; }
    static const char * update_coeff()   { return "update_coeff"; }
    static const char * cfl()            { return "cfl"; }
    static const char * time_accurate()  { return "time_accurate"; }
  }; // Tags

  /// Contructor
  /// @param name of the component
  FlowSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~FlowSolver();

  /// Get the class name
  static std::string type_name () { return "FlowSolver"; }

  virtual void solve();

  /// @name SIGNALS
  //@{

  /// creates a boundary condition
  void signal_create_bc_action( Common::SignalArgs& xml );
  void signature_create_bc_action( Common::SignalArgs& xml );

  void signal_create_inner_action( Common::SignalArgs& xml );
  void signature_create_inner_action( Common::SignalArgs& xml );

  void signal_create_solve( Common::SignalArgs& xml );
  void signature_create_solve( Common::SignalArgs& xml );

  //@} END SIGNALS


  virtual Common::CAction& create_solve(const std::string& name, const std::string& solve_builder_name);
  virtual Common::CAction& create_setup(const std::string& name, const std::string& setup_builder_name);
  virtual Common::CAction& create_bc_action(const std::string& name, const std::string& bc_action_builder_name, const std::vector<boost::shared_ptr<Mesh::CRegion const> >& regions);
  virtual Common::CAction& create_bc_action(const std::string& name, const std::string& bc_action_builder_name, const Mesh::CRegion& region);
  virtual Common::CAction& create_inner_action(const std::string& name, const std::string& inner_action_builder_name, const std::vector<boost::shared_ptr<Mesh::CRegion const> >& regions);
  virtual Common::CAction& create_inner_action(const std::string& name, const std::string& inner_action_builder_name, const Mesh::CRegion& region);

private: // functions

  void setup();

  void auto_config(Component& component);

private: // data

  boost::weak_ptr<Common::CAction> m_setup;
  boost::weak_ptr<Common::CAction> m_solve;
  boost::weak_ptr<Common::CAction> m_bc;
  boost::weak_ptr<Common::CAction> m_inner;

  boost::weak_ptr<Physics::PhysModel>     m_physical_model;
  boost::weak_ptr<Solver::CTime>          m_time;
  boost::weak_ptr<Mesh::CMesh>            m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_FlowSolver_hpp
