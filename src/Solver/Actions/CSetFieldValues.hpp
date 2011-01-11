// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CSetFieldValues_hpp
#define CF_Solver_Actions_CSetFieldValues_hpp

#include "Mesh/CFieldElements.hpp"
#include "Mesh/CNodes.hpp"

#include "Solver/Actions/CNodeOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  template <typename T> class CTable;
  class CFieldElements;
}
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CSetFieldValues : public CNodeOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CSetFieldValues> Ptr;
  typedef boost::shared_ptr<CSetFieldValues const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSetFieldValues ( const std::string& name );

  /// Virtual destructor
  virtual ~CSetFieldValues() {};

  /// Get the class name
  static std::string type_name () { return "CSetFieldValues"; }

  /// Set the loop_helper
  void create_loop_helper (Mesh::CElements& geometry_elements );
	
  /// execute the action
  virtual void execute ();
	
	/// @return the nodes to loop over
  virtual Mesh::CList<Uint>& loop_list () const;
	
private: // data
	
  struct LoopHelper
  {
    LoopHelper(Mesh::CElements& geometry_elements, CLoopOperation& op) :
		field_data(geometry_elements.get_field_elements(op.properties()["Field"].value<std::string>()).data()),
		coordinates(geometry_elements.get_field_elements(op.properties()["Field"].value<std::string>()).nodes().coordinates()),
		used_nodes(geometry_elements.get_field_elements(op.properties()["Field"].value<std::string>()).used_nodes())
    { }
    Mesh::CTable<Real>& field_data;
    Mesh::CTable<Real>& coordinates;
    Mesh::CList<Uint>& used_nodes;
  };
	
  boost::shared_ptr<LoopHelper> m_loop_helper;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CSetFieldValues_hpp
