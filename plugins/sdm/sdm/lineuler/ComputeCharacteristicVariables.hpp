// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_ComputeCharacteristicVariables_hpp
#define cf3_sdm_lineuler_ComputeCharacteristicVariables_hpp

#include "solver/Action.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace sdm {
namespace lineuler {

class sdm_lineuler_API ComputeCharacteristicVariables : public solver::Action
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeCharacteristicVariables> Ptr;
  typedef boost::shared_ptr<ComputeCharacteristicVariables const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeCharacteristicVariables ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeCharacteristicVariables() {};

  /// Get the class name
  static std::string type_name () { return "ComputeCharacteristicVariables"; }

  /// execute the action
  virtual void execute ();

private: // data

  Handle<mesh::Field> m_characteristics;
  Handle<mesh::Field> m_char_resid;
  Handle<mesh::Field> m_dchardn;
  Handle<mesh::Field> m_dchards;
  Handle<mesh::Field> m_char_convection;

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_ComputeCharacteristicVariables_hpp
