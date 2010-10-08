// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CSchemeLDA_hpp
#define CF_Mesh_CSchemeLDA_hpp

#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"

#include "Actions/CElementOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CSchemeLDA : public CElementOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CSchemeLDA> Ptr;
  typedef boost::shared_ptr<CSchemeLDA const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSchemeLDA ( const CName& name );

  /// Virtual destructor
  virtual ~CSchemeLDA() {};

  /// Get the class name
  static std::string type_name () { return "CSchemeLDA"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options );

  /// configure m_solution_field
  void trigger_SolutionField();

  /// configure m_residual_field
  void trigger_ResidualField();

  /// Set the loop_helper
  void set_loophelper (CElements& geometry_elements )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*m_solution_field, *m_residual_field, geometry_elements ) );
  }

  /// execute the action
  virtual void execute ();
    
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& solution_field, CField& residual_field, CElements& geometry_elements) :
      solution_field_elements(geometry_elements.get_field_elements(solution_field.field_name())),
      residual_field_elements(geometry_elements.get_field_elements(residual_field.field_name())),
      solution(solution_field_elements.data()),
      residual(residual_field_elements.data()),
      // Assume coordinates and connectivity_table are the same for solution and residual (pretty safe)
      coordinates(solution_field_elements.coordinates()),
      connectivity_table(solution_field_elements.connectivity_table())
    { }
    CFieldElements& solution_field_elements;
    CFieldElements& residual_field_elements;
    CArray& solution;
    CArray& residual;
    CArray& coordinates;
    CTable& connectivity_table;
  };

  boost::shared_ptr<LoopHelper> data;

  /// The field set by configuration, to perform action on
  CField::Ptr m_solution_field;
  CField::Ptr m_residual_field;

};

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSchemeLDA_hpp
