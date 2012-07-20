// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_DomainDiscretization_hpp
#define cf3_sdm_DomainDiscretization_hpp

#include "mesh/Region.hpp"

#include "solver/ActionDirector.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace mesh { class Cells; }
namespace sdm {

class Term;

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API DomainDiscretization : public cf3::solver::ActionDirector {

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

  void update();

  bool loop_cells(const Handle<mesh::Cells const>& cells);

  void compute_element(const Uint elem_idx);

private:

  Handle< common::ActionDirector > m_terms;   ///< set of terms
  std::map< Handle<mesh::Cells const> , std::vector< Handle<Term> > > m_terms_per_cells;
  Handle<mesh::Cells const> m_cells;
  std::vector< Handle<Term> > m_terms_for_cells;

};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_DomainDiscretization_hpp
