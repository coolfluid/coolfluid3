// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RungeKutta_UpdateSolution_hpp
#define CF_RungeKutta_UpdateSolution_hpp

#include "Common/CAction.hpp"

#include "RungeKutta/LibRungeKutta.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class Field; }
namespace RungeKutta {

class RungeKutta_API UpdateSolution : public Common::CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<UpdateSolution> Ptr;
  typedef boost::shared_ptr<UpdateSolution const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  UpdateSolution ( const std::string& name );

  /// Virtual destructor
  virtual ~UpdateSolution() {}

  /// Get the class name
  static std::string type_name () { return "UpdateSolution"; }

  /// execute the action
  virtual void execute ();

  void set_coefficients(const Real& alpha, const Real& beta)
  {
    m_alpha = alpha;
    m_beta = beta;
  }

private: // data

  boost::weak_ptr<Mesh::Field> m_solution;
  boost::weak_ptr<Mesh::Field> m_solution_backup;
  boost::weak_ptr<Mesh::Field> m_residual;
  boost::weak_ptr<Mesh::Field> m_update_coeff;

  Real m_alpha;
  Real m_beta;
};

////////////////////////////////////////////////////////////////////////////////

} // RungeKutta
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RungeKutta_UpdateSolution_hpp
