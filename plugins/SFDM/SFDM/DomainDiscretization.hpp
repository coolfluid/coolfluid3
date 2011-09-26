// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_DomainDiscretization_hpp
#define CF_SFDM_DomainDiscretization_hpp

#include "Solver/ActionDirector.hpp"

#include "SFDM/LibSFDM.hpp"

namespace CF {
namespace SFDM {

class Term;

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API DomainDiscretization : public CF::Solver::ActionDirector {

public: // typedefs

  typedef boost::shared_ptr<DomainDiscretization> Ptr;
  typedef boost::shared_ptr<DomainDiscretization const> ConstPtr;

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
                     const std::vector<Common::URI>& regions = std::vector<Common::URI>() );

  /// @name SIGNALS
  //@{

  /// creates a term
  void signal_create_term( Common::SignalArgs& args );
  /// signature for @see signal_create_term
  void signature_signal_create_term( Common::SignalArgs& args );

  //@} END SIGNALS

private:

  Common::CActionDirector::Ptr m_terms;   ///< set of terms

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // CF

#endif // CF_SFDM_DomainDiscretization_hpp
