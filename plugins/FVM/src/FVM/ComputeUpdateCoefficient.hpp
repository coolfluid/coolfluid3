// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ComputeUpdateCoefficient_hpp
#define CF_FVM_ComputeUpdateCoefficient_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"
#include "FVM/RoeFluxSplitter.hpp"

#include "Mesh/CCellFaces.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CTable.hpp"
#include "Common/Foreach.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class CField2; }
namespace Solver { class CTime;   }
namespace FVM {

class FVM_API ComputeUpdateCoefficient : public Common::CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeUpdateCoefficient> Ptr;
  typedef boost::shared_ptr<ComputeUpdateCoefficient const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeUpdateCoefficient ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeUpdateCoefficient() {};

  /// Get the class name
  static std::string type_name () { return "ComputeUpdateCoefficient"; }
  
  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_update_coeff();
  void config_advection();
  void config_volume();
  void config_time();
  
private: // data
  
  boost::weak_ptr<Mesh::CField2> m_update_coeff;
  boost::weak_ptr<Mesh::CField2> m_advection;
  boost::weak_ptr<Mesh::CField2> m_volume;
  boost::weak_ptr<Solver::CTime> m_time;
  
  bool m_time_accurate;
  Real m_CFL;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ComputeUpdateCoefficient_hpp
