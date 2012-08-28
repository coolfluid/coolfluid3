// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_BoundaryConditions_hpp
#define cf3_sdm_BoundaryConditions_hpp

#include "mesh/Region.hpp"

#include "solver/ActionDirector.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace sdm {

  class SharedCaches;
  class BC;

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API BoundaryConditions : public cf3::solver::ActionDirector {

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
                                 const std::vector< Handle<mesh::Region> >& regions );

  /// @name SIGNALS
  //@{

  /// creates a term
  void signal_create_boundary_condition( common::SignalArgs& args );
  /// signature for @see signal_create_term
  void signature_signal_create_boundary_condition( common::SignalArgs& args );

  //@} END SIGNALS

private:

  std::map< Handle<mesh::Region const> , Handle<BC> > m_bc_per_region;

  Handle<mesh::Field>                m_solution;
  Handle<SharedCaches>               m_shared_caches;

};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_BoundaryConditions_hpp
