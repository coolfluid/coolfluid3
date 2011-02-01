// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_FiniteVolume_hpp
#define CF_FVM_FiniteVolume_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

#include "Solver/CDiscretization.hpp"

#include "FVM/LibFVM.hpp"
namespace CF {

  namespace Solver {
  namespace Actions { 
    class CLoop; 
  }
  }

namespace FVM {

////////////////////////////////////////////////////////////////////////////////

/// FVM component class
/// @author Tiago Quintino
/// @author Willem Deconinck
class FVM_API FiniteVolume : public Solver::CDiscretization {

public: // typedefs

  typedef boost::shared_ptr<FiniteVolume> Ptr;
  typedef boost::shared_ptr<FiniteVolume const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  FiniteVolume ( const std::string& name );

  /// Virtual destructor
  virtual ~FiniteVolume();

  /// Get the class name
  static std::string type_name () { return "FiniteVolume"; }

  // functions specific to the FiniteVolume component
  
  /// computes the discrete rhs of the PDE
  virtual void compute_rhs();

  /// @name SIGNALS
  //@{

  /// creates a boundary condition
  void create_bc( Common::XmlNode& xml );

  //@} END SIGNALS

private: // functions
  
  /// function triggered when regions option is updated
  void trigger_Regions();
  
  void configure_solution();
  void configure_residual();
  void configure_update_coeff();
  
private: // data

  boost::shared_ptr<Solver::Actions::CForAllFaces> m_face_loop;
  
  Common::CLink::Ptr m_solution;
  
  Common::CLink::Ptr m_residual;
  
  Common::CLink::Ptr m_update_coeff;
  
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_FiniteVolume_hpp
