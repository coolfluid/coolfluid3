// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CModel_hpp
#define CF_Solver_CModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
  namespace Common { class CGroup; }
  namespace Mesh   { class CDomain; }
namespace Solver {

  class CPhysicalModel;
  class CSolver;
  class CTime;

////////////////////////////////////////////////////////////////////////////////

/// CModel is the top most component on a simulation structure
/// CModel now stores:
/// - PhysicalModel
/// - Iterative solver
/// - Discretization
/// @author Martin Vymazal
class Solver_API CModel : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CModel> Ptr;
  typedef boost::shared_ptr<CModel const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CModel ( const std::string& name );

  /// Virtual destructor
  virtual ~CModel();

  /// Get the class name
  static std::string type_name () { return "CModel"; }

  /// creates a domain in this model
  virtual Mesh::CDomain& create_domain( const std::string& name );

  /// creates a domain in this model
  virtual CTime& create_time( const std::string& name );

  /// create physics
  virtual CPhysicalModel& create_physics( const std::string& name );

  /// create physics
  virtual CSolver& create_solver( const std::string& name );

  /// gets the domain from this model
  virtual Mesh::CDomain& domain();

  /// gets the domain from this model
  virtual CTime& time();

  /// gets the physics from this model
  virtual CPhysicalModel& physics();

  /// gets the solver from this model
  virtual CSolver& solver();

  /// gets the solver from this model
  virtual Common::CGroup& tools();

  /// Simulates this model
  virtual void simulate();

  /// @name SIGNALS
  //@{

  /// Signature of create physics signal @see signal_create_domain
  void signature_create_physics ( Common::SignalArgs& node );
  /// Signal to create the physics
  void signal_create_physics ( Common::SignalArgs& node );

  /// Signature of create domain signal @see signal_create_domain
  void signature_create_domain ( Common::SignalArgs& node );
  /// Signal to create a domain and load a mesh into it
  void signal_create_domain ( Common::SignalArgs& node );

  /// Signature of create domain signal @see signal_create_domain
  void signature_create_solver ( Common::SignalArgs& node );
  /// Signal to create a domain and load a mesh into it
  void signal_create_solver ( Common::SignalArgs& node );

  /// Signal to start simulating
  void signal_simulate ( Common::SignalArgs& node );

  //@} END SIGNALS


protected:

  /// path to working directory
  Common::URI m_working_dir;

  /// path to results directory
  Common::URI m_results_dir;

  boost::shared_ptr<Common::CGroup> m_tools;

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CModel_hpp
