// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_FiniteVolume_hpp
#define CF_FVM_FiniteVolume_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/CAction.hpp"

#include "Solver/CDiscretization.hpp"

#include "FVM/LibFVM.hpp"
namespace CF {
  namespace Solver {
    namespace Actions { class CLoop; }
  }
  namespace Mesh { class CRegion;  }

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
  void signal_create_bc( Common::XmlNode& xml );


  Common::CAction& create_bc(const std::string& name, const std::vector<boost::shared_ptr<Mesh::CRegion> >& regions, const std::string& bc_builder_name);
  Common::CAction& create_bc(const std::string& name, const Mesh::CRegion& region, const std::string& bc_builder_name);
  
  //@} END SIGNALS
  
private: // functions

  void on_config_mesh();

private: // data
  
  Common::CAction::Ptr m_apply_bcs;
  
  Common::CAction::Ptr m_compute_rhs;

};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_FiniteVolume_hpp
