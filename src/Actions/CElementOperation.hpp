// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CElementOperation_hpp
#define CF_Mesh_CElementOperation_hpp

#include "Mesh/CField.hpp"
#include "Actions/CAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CElementOperation : public CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CElementOperation> Ptr;
  typedef boost::shared_ptr<CElementOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CElementOperation ( const CName& name );

  /// Virtual destructor
  virtual ~CElementOperation() {};

  /// Get the class name
  static std::string type_name () { return "CElementOperation"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options );
  
  virtual void set_loophelper ( CElements& geometry_elements ) = 0;
  
  void set_element_idx ( const Uint elem_idx ) { m_elm_idx = elem_idx; }
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

protected: // data

  Uint m_elm_idx;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElementOperation_hpp
