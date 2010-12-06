// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CTakeStep_hpp
#define CF_Mesh_CTakeStep_hpp

#include "Common/ComponentPredicates.hpp"

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"

#include "Actions/CLoopOperation.hpp"

#include "Solver/LibSolver.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_API CTakeStep : public Actions::CLoopOperation
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

  /// Set the loop_helper
  void set_loophelper (Mesh::CElements& geometry_elements );
	
	/// @return the nodes to loop over
  virtual Mesh::CList<Uint>& loop_list ();
	
  /// execute the action
  virtual void execute ();
		
private: // data
	
  struct LoopHelper
  {
    LoopHelper(Mesh::CElements& geometry_elements, CLoopOperation& op) :
		solution(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).data()),
		residual(geometry_elements.get_field_elements(op.properties()["ResidualField"].value<std::string>()).data()),
		inverse_updatecoeff(geometry_elements.get_field_elements(op.properties()["InverseUpdateCoeff"].value<std::string>()).data()),
		node_list(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).node_list())
    { }
    Mesh::CTable<Real>& solution;
    Mesh::CTable<Real>& residual;
    Mesh::CTable<Real>& inverse_updatecoeff;
    Mesh::CList<Uint>& node_list;
  };
	
  boost::shared_ptr<LoopHelper> data;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTakeStep_hpp
