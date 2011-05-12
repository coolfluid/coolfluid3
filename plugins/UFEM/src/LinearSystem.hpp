// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_LinearSystem_hpp
#define CF_UFEM_LinearSystem_hpp

#include "Common/OptionURI.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/CSolver.hpp"

#include "Solver/Actions/CActionDirector.hpp"

#include "Solver/Actions/Proto/CProtoElementsAction.hpp"
#include "Solver/Actions/Proto/CProtoNodesAction.hpp"
#include "Solver/Actions/Proto/PhysicalModel.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "LibUFEM.hpp"

namespace CF {

namespace UFEM {

/// A method that solves a linear system to obtain the problem solution. The system is set up based on a proto expression
class UFEM_API LinearSystem : public Solver::CSolver
{
public: // typedefs

  typedef boost::shared_ptr<LinearSystem> Ptr;
  typedef boost::shared_ptr<LinearSystem const> ConstPtr;

public: // functions

  /// Customizable solver phases
  enum PhasesT
  {
    POST_INIT=0,
    ASSEMBLY=1,
    POST_SOLVE=2,
    POST_INCREMENT=3
  };
  
  /// Contructor
  /// @param name of the component
  LinearSystem ( const std::string& name );

  /// Get the class nameadd
  static std::string type_name () { return "LinearSystem"; }

  /// CSolver implementation
  virtual void solve();

  /// The linear system solver
  Solver::CEigenLSS& lss();
  
  /// Append an action with the given name that operates on elements to the appropriate phase
  template<typename ExprT>
  void append_elements_action(const std::string& name, const PhasesT phase, const ExprT& expr)
  {
    append_action( Solver::Actions::Proto::build_elements_action(name, *this, *this, m_physical_model, expr, m_region_path.lock()), phase );
  }

  /// @name SIGNALS
  //@{

  /// Signal to add Dirichlet boundary conditions
  void signal_add_dirichlet_bc( Common::SignalArgs& node );

  /// Signature for the add_dirichlet_bc ssignal
  void signature_add_dirichlet_bc( Common::SignalArgs& node);

  /// Signal to add an intial condition
  void signal_add_initial_condition( Common::SignalArgs& node );

  /// Signature for add_initial_condition
  void signature_add_initial_condition(Common::SignalArgs& node);

  /// Signal to run the intialization phase
  void signal_initialize_fields(Common::SignalArgs& node);

  //@} END SIGNALS
protected:
  /// Override this to create the required CActions
  virtual void add_actions() = 0;

  /// Overridable implementation that is called when the action is run
  virtual void on_solve();
  
  /// Adds a configurable constant
  Solver::Actions::Proto::StoredReference<Real> add_configurable_constant(const std::string& name, const std::string& description, const Real default_value);

private:
  /// Append an action to a phase
  void append_action(Common::CAction& action, const PhasesT phase);
  
  /// Temp storage for configurable constants that are added
  typedef std::map<std::string, Common::Component::Ptr> ConstantsT;
  ConstantsT m_configurable_constants;

  /// Called when the LSS is changed
  void trigger_lss_set();

  /// Path to the linear system
  boost::weak_ptr<Common::OptionURI> m_lss_path;

  /// Linear system
  boost::weak_ptr<Solver::CEigenLSS> m_lss;
  
  /// Region to solve over
  boost::weak_ptr<Common::OptionURI> m_region_path;
  
  /// CActionDirectors that take care of running customizable actions during certain phases of the solution. Indexed by PhasesT
  boost::weak_ptr<Solver::Actions::CActionDirector> m_phases[4];
  
  Solver::Actions::Proto::PhysicalModel m_physical_model;
};

} // UFEM
} // CF


#endif // CF_UFEM_LinearSystem_hpp
