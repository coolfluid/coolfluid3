// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ComputeFlux_hpp
#define CF_FVM_ComputeFlux_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CFieldView;
}
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API ComputeFlux : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeFlux> Ptr;
  typedef boost::shared_ptr<ComputeFlux const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeFlux ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeFlux() {};

  /// Get the class name
  static std::string type_name () { return "ComputeFlux"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();
  void config_residual();

  void trigger_elements();
  
private: // data
  
  boost::shared_ptr<Mesh::CFieldView> m_residual;
  boost::shared_ptr<Mesh::CFieldView> m_solution;

};

/////////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ComputeFlux_hpp
