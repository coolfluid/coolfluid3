// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CForAllNodes_hpp
#define CF_Mesh_CForAllNodes_hpp


#include "Actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;
using namespace CF::Common::String;
namespace CF {
	namespace Mesh{
		class CElements;
	}
namespace Actions {

/////////////////////////////////////////////////////////////////////////////////////

class Actions_API CForAllNodes : public CLoop
{
public: // typedefs

  typedef boost::shared_ptr< CForAllNodes > Ptr;
  typedef boost::shared_ptr< CForAllNodes const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllNodes ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllNodes() {}

  /// Get the class name
  static std::string type_name () { return "CForAllNodes"; }

  /// Configuration Options
  virtual void define_config_properties ();

  // functions specific to the CForAllNodes component

  virtual void execute();

	
private: // helper functions

  /// regists all the signals declared in this class
  virtual void define_signals () {}

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CForAllNodes_hpp
