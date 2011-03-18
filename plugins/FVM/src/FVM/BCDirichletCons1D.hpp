// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_BCDirichletCons1D_hpp
#define CF_FVM_BCDirichletCons1D_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CConnectedFieldView;
}
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API BCDirichletCons1D : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BCDirichletCons1D> Ptr;
  typedef boost::shared_ptr<BCDirichletCons1D const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BCDirichletCons1D ( const std::string& name );

  /// Virtual destructor
  virtual ~BCDirichletCons1D() {};

  /// Get the class name
  static std::string type_name () { return "BCDirichletCons1D"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();

  void trigger_elements();
  
private: // data
  
  enum {FIRST=0};
  
  Mesh::CConnectedFieldView m_connected_solution;
  
  Real m_rho;
  Real m_u;
  Real m_p;
  Real m_gm1;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_BCDirichletCons1D_hpp
