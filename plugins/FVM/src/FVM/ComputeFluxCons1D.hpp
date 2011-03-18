// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ComputeFluxCons1D_hpp
#define CF_FVM_ComputeFluxCons1D_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"
#include "FVM/RoeFluxSplitterCons1D.hpp"

#include "Mesh/CCellFaces.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CTable.hpp"
#include "Common/Foreach.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CFieldView;
  class CConnectedFieldView;
}
namespace FVM {
  
///////////////////////////////////////////////////////////////////////////////////////

class FVM_API ComputeFluxCons1D : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeFluxCons1D> Ptr;
  typedef boost::shared_ptr<ComputeFluxCons1D const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeFluxCons1D ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeFluxCons1D() {};

  /// Get the class name
  static std::string type_name () { return "ComputeFluxCons1D"; }

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
  
  boost::shared_ptr<RoeFluxSplitterCons1D> m_fluxsplitter;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ComputeFluxCons1D_hpp
