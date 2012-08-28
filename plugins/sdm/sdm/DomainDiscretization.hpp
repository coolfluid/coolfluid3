// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_DomainDiscretization_hpp
#define cf3_sdm_DomainDiscretization_hpp

#include "mesh/Region.hpp"

#include "common/Action.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace mesh { class Cells; class Entities; class Space; class Field; }
namespace sdm {

class SharedCaches;
class Term;

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API DomainDiscretization : public cf3::common::Action {

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

  Term& create_term( const std::string& name,const std::string& type);

  /// @name SIGNALS
  //@{

  /// creates a term
  void signal_create_term( common::SignalArgs& args );
  /// signature for @see signal_create_term
  void signature_signal_create_term( common::SignalArgs& args );

  //@} END SIGNALS

  void update();

  bool loop_cells(const Handle<mesh::Entities const>& cells);

  void compute_element(const Uint elem_idx);

private:

  Handle<mesh::Cells const> m_cells;
  Handle<mesh::Space const> m_space;
  std::vector< Handle<Term> > m_terms_vector;

  Handle<mesh::Field> m_solution;
  Handle<mesh::Field> m_residual;
  Handle<mesh::Field> m_wave_speed;

  Handle<SharedCaches>               m_shared_caches;

};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_DomainDiscretization_hpp
