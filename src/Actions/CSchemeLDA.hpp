// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CSchemeLDA_hpp
#define CF_Mesh_CSchemeLDA_hpp

#include <boost/assign.hpp>

#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"

#include "Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Mesh;
using namespace boost::assign;
namespace CF {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Actions_API CSchemeLDA : public CLoopOperation
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

  /// Set the loop_helper
  void set_loophelper (CElements& geometry_elements );
	
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
      // Assume coordinates and connectivity_table are the same for solution and residual (pretty safe)
      coordinates(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).coordinates()),
      connectivity_table(geometry_elements.get_field_elements(op.properties()["SolutionField"].value<std::string>()).connectivity_table())
    { }
    CArray& solution;
    CArray& residual;
    CArray& inverse_updatecoeff;
    CArray& coordinates;
    CTable& connectivity_table;
  };

  boost::shared_ptr<LoopHelper> data;
  
  Uint nb_q;
  Real w;
  std::vector<RealVector> mapped_coords;

};

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSchemeLDA_hpp
