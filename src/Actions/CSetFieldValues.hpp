// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CSetFieldValues_hpp
#define CF_Mesh_CSetFieldValues_hpp

#include "Mesh/CField.hpp"
#include "Actions/CAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CSetFieldValues : public CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CSetFieldValues> Ptr;
  typedef boost::shared_ptr<CSetFieldValues const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSetFieldValues ( const CName& name );

  /// Virtual destructor
  virtual ~CSetFieldValues() {};

  /// Get the class name
  static std::string type_name () { return "CSetFieldValues"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options );

  /// execute the action
  virtual void execute ();
  
  /// configure the field
  void trigger_Field();
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  /// The field set by configuration, to perform action on
  CField::Ptr m_field;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSetFieldValues_hpp
