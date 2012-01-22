// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_IterativeSolver_hpp
#define cf3_sdm_IterativeSolver_hpp

#include "solver/Action.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace common { class ActionDirector; }
namespace solver { class Time; }
namespace mesh   { class Field; }
namespace sdm {


/////////////////////////////////////////////////////////////////////////////////////

class sdm_API IterativeSolver : public solver::Action {

public: // functions

  /// Contructor
  /// @param name of the component
  IterativeSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~IterativeSolver() {}

  /// Get the class name
  static std::string type_name () { return "IterativeSolver"; }

  common::ActionDirector& pre_update()    { return *m_pre_update; }
  common::ActionDirector& post_update()   { return *m_post_update; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

protected: // functions

  /// Link the solution field, the residual field and the update-coefficient field
  virtual void link_fields();

  /// raises the event when iteration done
  void raise_iteration_done();

protected: // data

  Handle<mesh::Field> m_solution;        //!< Solution field
  Handle<mesh::Field> m_residual;        //!< Residual field
  Handle<mesh::Field> m_update_coeff;    //!< Update coefficient field
  Handle<solver::Time> m_time;           //!< Time component

  /// set of actions called every iteration before non-linear solve
  Handle<common::ActionDirector> m_pre_update;
  /// set of actions called every iteration after non-linear solve
  Handle<common::ActionDirector> m_post_update;

};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_IterativeSolver_hpp
