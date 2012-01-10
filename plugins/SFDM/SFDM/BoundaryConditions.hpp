// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_BoundaryConditions_hpp
#define cf3_SFDM_BoundaryConditions_hpp

#include "mesh/Region.hpp"

#include "solver/ActionDirector.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace SFDM {

class BC;

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API BoundaryConditions : public cf3::solver::ActionDirector {

public: // typedefs

public: // functions
  /// Contructor
  /// @param name of the component
  BoundaryConditions ( const std::string& name );

  /// Virtual destructor
  virtual ~BoundaryConditions() {}

  /// Get the class name
  static std::string type_name () { return "BoundaryConditions"; }

  /// execute the action
  virtual void execute ();

  BC& create_boundary_condition( const std::string& type,
                                 const std::string& name,
                                 const std::vector<common::URI>& regions = std::vector<common::URI>() );

  /// @name SIGNALS
  //@{

  /// creates a term
  void signal_create_boundary_condition( common::SignalArgs& args );
  /// signature for @see signal_create_term
  void signature_signal_create_boundary_condition( common::SignalArgs& args );

  //@} END SIGNALS

private:

  Handle< common::ActionDirector > m_bcs;   ///< set of terms
  std::map< Handle<mesh::Region const> , Handle<BC> > m_bc_per_region;

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3

#endif // cf3_SFDM_BoundaryConditions_hpp
