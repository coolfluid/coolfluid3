// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ComputeFlux_hpp
#define CF_FVM_ComputeFlux_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/RiemannSolver.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CFieldView;
  class CConnectedFieldView;
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
  void config_wave_speed();
  void config_area();
  void config_normal();

  void trigger_elements();
  
private: // data
  
  Mesh::CConnectedFieldView m_connected_residual;
  Mesh::CConnectedFieldView m_connected_solution;
  Mesh::CConnectedFieldView m_connected_wave_speed;
  Mesh::CScalarFieldView    m_face_area;
  Mesh::CFieldView          m_face_normal;
  
  RealVector m_flux;
  Real m_wave_speed_left;
  Real m_wave_speed_right;
  RealVector m_normal;
  RealVector m_state_L;
  RealVector m_state_R;
  
  enum {LEFT=0,RIGHT=1};
  
  boost::shared_ptr<RiemannSolver> m_fluxsplitter;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ComputeFlux_hpp
