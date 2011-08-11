// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_DomainDiscretization_hpp
#define CF_RDM_DomainDiscretization_hpp

#include "Solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
namespace RDM {

class CellTerm;
class FaceTerm;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API DomainDiscretization : public CF::Solver::ActionDirector {

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

  RDM::CellTerm& create_cell_term( const std::string& type,
                                   const std::string& name,
                                   const std::vector<Common::URI>& regions );

  RDM::FaceTerm& create_face_term( const std::string& type,
                                   const std::string& name,
                                   const std::vector<Common::URI>& regions );


  /// @name SIGNALS
  //@{

  /// creates a cell term
  void signal_create_cell_term( Common::SignalArgs& args );
  /// signature for @see signal_create_cell_term
  void signature_signal_create_cell_term( Common::SignalArgs& args );

  /// creates a face term
  void signal_create_face_term( Common::SignalArgs& args );
  /// signature for @see signal_create_face_term
  void signature_signal_create_face_term( Common::SignalArgs& args );

  //@} END SIGNALS

private:

  Common::CActionDirector::Ptr m_face_terms;   ///< set of face terms

  Common::CActionDirector::Ptr m_cell_terms;   ///< set of cell terms

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF

#endif // CF_RDM_DomainDiscretization_hpp
