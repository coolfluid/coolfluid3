// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CForAllElements_hpp
#define CF_Mesh_CForAllElements_hpp


#include "Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;
using namespace CF::Common::String;
namespace CF {
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Actions_API CForAllElements : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllElements > Ptr;
  typedef boost::shared_ptr< CForAllElements const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllElements ( const CName& name );

  /// Virtual destructor
  virtual ~CForAllElements() {}

  /// Get the class name
  static std::string type_name () { return "CForAllElements"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options );

  // functions specific to the CForAllElements component

  virtual void execute();

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CForAllElements_hpp
