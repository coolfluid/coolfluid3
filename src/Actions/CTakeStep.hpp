// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CTakeStep_hpp
#define CF_Mesh_CTakeStep_hpp

#include "Common/ComponentPredicates.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CArray.hpp"
#include "Actions/CAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;
using namespace CF::Common;

namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CTakeStep : public CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CTakeStep> Ptr;
  typedef boost::shared_ptr<CTakeStep const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CTakeStep ( const CName& name );

  /// Virtual destructor
  virtual ~CTakeStep() {};

  /// Get the class name
  static std::string type_name () { return "CTakeStep"; }

  /// Configuration Options
  static void defineConfigProperties ( PropertyList& options );

  /// execute the action
  virtual void execute ();
  
  void execute_impl();
  
  void go_deeper(CField& driving_field, std::vector<CField::Ptr>& driven_fields);
  
  /// configure m_solution_field
  void trigger_SolutionField();

  /// configure m_residual_field
  void trigger_ResidualField();
  
  /// configure m_inverseUpdateCoeff
  void trigger_InverseUpdateCoeff();
    
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  /// The field set by configuration, to perform action on
  CField::Ptr m_solution_field;
  CField::Ptr m_residual_field;
  CField::Ptr m_inverseUpdateCoeff;

  struct LoopHelper
  {
    LoopHelper(CField& solution_field, CField& residual_field, CField& inverse_updateCoeff_field) :
      solution            (get_tagged_component_typed<CArray>(solution_field, "field_data")),
      residual            (get_tagged_component_typed<CArray>(residual_field, "field_data")),
      inverse_updatecoeff (get_tagged_component_typed<CArray>(inverse_updateCoeff_field, "field_data"))
    { }
    CArray& solution;
    CArray& residual;
    CArray& inverse_updatecoeff;
  };

  boost::shared_ptr<LoopHelper> data;

  Uint m_point_idx;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CTakeStep_hpp
