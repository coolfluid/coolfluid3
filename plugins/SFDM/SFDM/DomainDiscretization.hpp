// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_DomainDiscretization_hpp
#define cf3_SFDM_DomainDiscretization_hpp

#include "mesh/Region.hpp"

#include "solver/ActionDirector.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {
namespace SFDM {

class Term;

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API DomainDiscretization : public cf3::solver::ActionDirector {

public: // typedefs

public: // functions
  /// Contructor
  /// @param name of the component
  DomainDiscretization ( const std::string& name );

  /// Virtual destructor
  virtual ~DomainDiscretization() {}

  /// Get the class name
  static std::string type_name () { return "DomainDiscretization"; }

  /// execute the action
  virtual void execute ();

  Term& create_term( const std::string& type,
                     const std::string& name,
                     const std::vector<common::URI>& regions = std::vector<common::URI>() );

  /// @name SIGNALS
  //@{

  /// creates a term
  void signal_create_term( common::SignalArgs& args );
  /// signature for @see signal_create_term
  void signature_signal_create_term( common::SignalArgs& args );

  //@} END SIGNALS

private:

  Handle< common::ActionDirector > m_terms;   ///< set of terms
  std::map< Handle<mesh::Region const> , std::vector< Handle<Term> > > m_terms_per_region;

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3

#endif // cf3_SFDM_DomainDiscretization_hpp
