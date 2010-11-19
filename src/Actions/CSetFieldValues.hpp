// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_CSetFieldValues_hpp
#define CF_Actions_CSetFieldValues_hpp

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

class Actions_API CSetFieldValues : public CLoopOperation
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
    LoopHelper(CElements& geometry_elements, CLoopOperation& op) :
		field_data(geometry_elements.get_field_elements(op.properties()["Field"].value<std::string>()).data()),
		coordinates(geometry_elements.get_field_elements(op.properties()["Field"].value<std::string>()).coordinates()),
		node_list(geometry_elements.get_field_elements(op.properties()["Field"].value<std::string>()).node_list())
    { }
    CArray& field_data;
    CArray& coordinates;
		CList<Uint>& node_list;
  };
	
  boost::shared_ptr<LoopHelper> data;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CSetFieldValues_hpp
