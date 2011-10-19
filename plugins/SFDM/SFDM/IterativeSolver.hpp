// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_IterativeSolver_hpp
#define cf3_SFDM_IterativeSolver_hpp

#include "Solver/Action.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace common { class CActionDirector; }
namespace Solver { class CTime; }
namespace Mesh   { class Field; }
namespace SFDM {


/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API IterativeSolver : public Solver::Action {

public: // typedefs

  typedef boost::shared_ptr<IterativeSolver> Ptr;
  typedef boost::shared_ptr<IterativeSolver const> ConstPtr;

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

  common::CActionDirector& pre_update()    { return *m_pre_update; }
  common::CActionDirector& post_update()   { return *m_post_update; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// raises the event when iteration done
  void raise_iteration_done();

  void config_rk_order();

  void link_fields();

private: // data

  std::vector<Real> m_alpha;
  std::vector<Real> m_beta;
  std::vector<Real> m_gamma;

  boost::weak_ptr<Mesh::Field> m_solution;
  boost::weak_ptr<Mesh::Field> m_solution_backup;
  boost::weak_ptr<Mesh::Field> m_residual;
  boost::weak_ptr<Mesh::Field> m_update_coeff;

  boost::weak_ptr<Solver::CTime> m_time;

  /// set of actions called every iteration before non-linear solve
  boost::shared_ptr<common::CActionDirector> m_pre_update;
  /// set of actions called every iteration after non-linear solve
  boost::shared_ptr<common::CActionDirector> m_post_update;

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3

#endif // CF3_SFDM_IterativeSolver_hpp
