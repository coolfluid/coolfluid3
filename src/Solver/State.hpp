// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_State_hpp
#define CF_Solver_State_hpp

#include "Common/Component.hpp"
#include "Math/MatrixTypes.hpp"
#include "Math/Defs.hpp"
#include "Solver/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the State
/// @author Willem Deconinck
class Solver_API State : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<State> Ptr;
  typedef boost::shared_ptr<State const> ConstPtr;

public: // functions

  /// Contructor
  State( const std::string& name = type_name()) : Component(name)
  {
  }

  /// Virtual destructor
  virtual ~State() {}

  /// Gets the Class name
  static std::string type_name() { return "State"; }

  virtual boost::shared_ptr<Physics> create_physics()
  {
    boost::shared_ptr<Physics> p(new Physics);
    return p;
  }

  const std::vector<std::string>& var_names() const { return m_var_names; }

  virtual Uint size() { return 0; }

  virtual void set_state( const RealVector& state,
                          Solver::Physics& p) {}

  virtual void get_state( Solver::Physics& p,
                          RealVector& state) {}


  virtual void compute_flux( Solver::Physics& p,
                             const RealVector& normal,
                             RealVector& flux) {}

  virtual void compute_fluxjacobian_right_eigenvectors( Solver::Physics& p,
                                                         const RealVector& normal,
                                                         RealMatrix& rv) {}

  virtual void compute_fluxjacobian_left_eigenvectors( Solver::Physics& p,
                                                        const RealVector& normal,
                                                        RealMatrix& lv) {}

  virtual void compute_fluxjacobian_eigenvalues( Solver::Physics& p,
                                                  const RealVector& normal,
                                                  RealVector& ev) {}

  virtual Real max_abs_eigen_value ( Solver::Physics& p,
                                 const RealVector& normal ) {return 0.;}

  virtual void linearize( std::vector<boost::shared_ptr<Solver::Physics> >& states,
                          Solver::Physics& p ) {}

protected:

  std::vector<std::string> m_var_names;

}; // State

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_State_hpp
