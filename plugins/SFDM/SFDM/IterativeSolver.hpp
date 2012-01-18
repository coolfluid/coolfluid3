// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_IterativeSolver_hpp
#define cf3_SFDM_IterativeSolver_hpp

#include "solver/Action.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace common { class ActionDirector; }
namespace solver { class CTime; }
namespace mesh   { class Field; }
namespace SFDM {


/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API IterativeSolver : public solver::Action {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  IterativeSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~IterativeSolver() {}

  /// Get the class name
  static std::string type_name () { return "IterativeSolver"; }

  /// execute the action
  virtual void execute ();

  common::ActionDirector& pre_update()    { return *m_pre_update; }
  common::ActionDirector& post_update()   { return *m_post_update; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// raises the event when iteration done
  void raise_iteration_done();

  void config_nb_stages();

  void link_fields();

private: // data

  std::vector<Real> m_alpha;
  std::vector<Real> m_beta;
  std::vector<Real> m_gamma;

  Handle<mesh::Field> m_solution;
  Handle<mesh::Field> m_solution_backup;
  Handle<mesh::Field> m_residual;
  Handle<mesh::Field> m_update_coeff;

  Handle<solver::CTime> m_time;

  /// set of actions called every iteration before non-linear solve
  Handle<common::ActionDirector> m_pre_update;
  /// set of actions called every iteration after non-linear solve
  Handle<common::ActionDirector> m_post_update;

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3

#endif // cf3_SFDM_IterativeSolver_hpp
