// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CDummyLoopOperation_hpp
#define CF_Actions_CDummyLoopOperation_hpp

#include "Mesh/CFieldElements.hpp"

#include "Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;

namespace CF {
	
	namespace Mesh {
		class CArray;
		class CFieldElements;
	}
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CDummyLoopOperation : public CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CDummyLoopOperation> Ptr;
  typedef boost::shared_ptr<CDummyLoopOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CDummyLoopOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~CDummyLoopOperation() {};

  /// Get the class name
  static std::string type_name () { return "CDummyLoopOperation"; }

  /// Configuration Options
  static void define_config_properties ( Common::PropertyList& options );

  /// Set the loop_helper
  void set_loophelper (CElements& geometry_elements );
	
  /// execute the action
  virtual void execute ();
	
	/// @return the nodes to loop over
	virtual CList<Uint>& loop_list ();
	
private: // helper functions
	
  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
	
private: // data
	
  struct LoopHelper
  {
    LoopHelper(CElements& geometry_elements) :
		node_list(geometry_elements.node_list())
    { }
		CList<Uint>& node_list;
  };
	
  boost::shared_ptr<LoopHelper> data;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CDummyLoopOperation_hpp
