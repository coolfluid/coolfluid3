// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_BCDirichlet_hpp
#define CF_FVM_BCDirichlet_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"
#include "FVM/RoeFluxSplitter.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CConnectedFieldView;
}
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API BCDirichlet : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BCDirichlet> Ptr;
  typedef boost::shared_ptr<BCDirichlet const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BCDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~BCDirichlet() {};

  /// Get the class name
  static std::string type_name () { return "BCDirichlet"; }

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

#endif // CF_FVM_BCDirichlet_hpp
