// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RungeKutta_RK_hpp
#define CF_RungeKutta_RK_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Math/MatrixTypes.hpp"
#include "RungeKutta/src/RungeKutta/LibRungeKutta.hpp"
#include "Solver/Action.hpp"

namespace CF {
namespace Common { class CGroupActions; }
namespace Mesh { class CField; }
namespace RungeKutta {
  class UpdateSolution;

////////////////////////////////////////////////////////////////////////////////

/// @brief Runge Kutta method
///
/// This method is based on the RKLS (Runge Kutta Low Storage) plugin found in
/// COOLFluiD version 2.
/// This action provides looping over runge kutta stages. @n
/// There are 2 slots to put custom actions inside:
/// - pre_update_actions
/// - post_update_actions
/// The update itself is delegated to the UpdateSolution component
/// time and dt are updated as well. (time update could be separate)
/// @author Willem Deconinck
class RungeKutta_API RK : public Solver::Action {

public: // typedefs

  typedef boost::shared_ptr<RK> Ptr;
  typedef boost::shared_ptr<RK const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RK ( const std::string& name );

  /// Virtual destructor
  virtual ~RK();

  /// Get the class name
  static std::string type_name () { return "RK"; }

  virtual void execute();

private:

  void config_stages();

private:

  Uint m_stages;

  boost::shared_ptr<Common::CGroupActions> m_pre_update;
  boost::shared_ptr<UpdateSolution> m_update;
  boost::shared_ptr<Common::CGroupActions> m_post_update;

  boost::weak_ptr<Mesh::CField> m_solution;
  boost::weak_ptr<Mesh::CField> m_residual;
  boost::weak_ptr<Mesh::CField> m_update_coeff;
  boost::weak_ptr<Mesh::CField> m_solution_backup;

  std::vector<Real> m_alpha;
  std::vector<Real> m_beta;
  std::vector<Real> m_gamma;

};

////////////////////////////////////////////////////////////////////////////////

} // RungeKutta
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RungeKutta_RK_hpp
