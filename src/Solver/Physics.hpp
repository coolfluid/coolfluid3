// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Physics_hpp
#define CF_Solver_Physics_hpp

#include "Common/Foreach.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/StringConversion.hpp"
#include "Solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the physics
/// @author Willem Deconinck
class Solver_API Physics {

public: // functions

  /// Typedef for the callback functions, used to compute variables
  typedef boost::function<Real (void)> ComputeFunction;

  /// Contructor
  Physics() {}

  /// Virtual destructor
  virtual ~Physics() {}

  /// Get the class name
  static std::string type_name () { return "Physics"; }

  /// Mark all the variables as not computed, e.g. when a new state is set
  void init()
  {
    m_variables_availability.assign(m_variables.size(),false);
  }

  /// Set a certain variable to a given value
  /// Typically this can be the state in primitive variables: rho, u, v, p
  void set_var(const Uint variable_id, const Real& value)
  {
    cf_assert(variable_id<m_variables.size());
    m_variables[variable_id] = value;
    m_variables_availability[variable_id]=true;

  }

  /// Fast access to the value of a variable
  Real var(const Uint variable_id) const
  {
    cf_assert(variable_id<m_variables.size());
    cf_assert_desc(Common::to_str(variable_id)+" not available",m_variables_availability[variable_id]);
    return m_variables[variable_id];
  }

  /// This is the trick. Compute variables only if needed,
  /// And compute all dependent variables that are needed.
  /// computation happens using a callback function, which can
  /// be set with set_compute_function()
  Real compute_var(const Uint variable_id)
  {
    cf_assert(variable_id<m_variables.size());
    if (this->must_compute(variable_id))
    {
      this->compute_dependent_vars(variable_id);
      this->set_var(variable_id,m_compute_functions[variable_id]());
    }
    return m_variables[variable_id];
  }

  /// An example callback function that throws an exception
  Real not_implemented()
  {
      throw Common::ValueNotFound(FromHere(),"Variable could not be computed.\n"
                                     "It should be provided using the set_var() function.");
      return 0.;
  }

  /// Get the variables to which a given variable depends on.
  std::vector<Uint>& var_deps(const Uint& variable_id)
  {
    return m_variables_dependency[variable_id];
  }

  /// Initialize the physical variables with a size
  void resize(const Uint size)
  {
      m_variables.resize(size);
      m_variables_availability.resize(size,false);
      m_variables_dependency.resize(size);
      m_compute_functions.resize(size, boost::bind( &Physics::not_implemented , this) );
  }

  /// Set the callback function
  void set_compute_function(const Uint variable_id, ComputeFunction func)
  {
    m_compute_functions[variable_id] = func;
  }

  Uint size() const { return m_variables.size(); }

  /// Check if a variable is already available
  bool must_compute(const Uint variable_id) const
  {
    cf_assert(variable_id<m_variables.size());
    return ( ! m_variables_availability[variable_id] );
  }

private: // functions

  /// Compute depending variables
  void compute_dependent_vars(const Uint variable_id)
  {
    boost_foreach(const Uint dependent_var_id, m_variables_dependency[variable_id])
    {
      set_var(dependent_var_id, compute_var(dependent_var_id));
    }
  }


private: // data

  // non constant variables, change with every new init()
  std::vector<Real>                 m_variables;
  std::vector<bool>                 m_variables_availability;


  // Constant variables, don't change per physics
  std::vector<std::vector<Uint> >        m_variables_dependency;
  std::vector<ComputeFunction>           m_compute_functions;

}; // CPhysicalModel

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Physics& p);

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Physics_hpp
