// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CTakeStep_hpp
#define CF_Mesh_CTakeStep_hpp

#include "Common/ComponentPredicates.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CList.hpp"
#include "Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;
using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CTakeStep : public CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CTakeStep> Ptr;
  typedef boost::shared_ptr<CTakeStep const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CTakeStep ( const std::string& name );

  /// Virtual destructor
  virtual ~CTakeStep() {};

  /// Get the class name
  static std::string type_name () { return "CTakeStep"; }

  /// Configuration Options
  static void define_config_properties ( PropertyList& options );

  /// Set the loop_helper
  void set_loophelper (CElements& geometry_elements );
	
	/// @return the nodes to loop over
	virtual CList<Uint>& loop_list ();
	
  /// execute the action
  virtual void execute ();
	
private: // helper functions
	
  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
	
private: // data
	
  struct LoopHelper
  {
    LoopHelper(CElements& geometry_elements, CLoopOperation& op) :
		solution(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).data()),
		residual(geometry_elements.get_field_elements(op.properties()["ResidualField"].value<std::string>()).data()),
		inverse_updatecoeff(geometry_elements.get_field_elements(op.properties()["InverseUpdateCoeff"].value<std::string>()).data()),
		node_list(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).node_list())
    { }
    CArray& solution;
    CArray& residual;
    CArray& inverse_updatecoeff;
		CList<Uint>& node_list;
  };
	
  boost::shared_ptr<LoopHelper> data;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTakeStep_hpp
