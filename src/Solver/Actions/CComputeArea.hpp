// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CComputeArea_hpp
#define CF_Solver_Actions_CComputeArea_hpp

#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CScalarFieldView;
}
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CComputeArea : public CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CComputeArea> Ptr;
  typedef boost::shared_ptr<CComputeArea const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CComputeArea ( const std::string& name );

  /// Virtual destructor
  virtual ~CComputeArea() {};

  /// Get the class name
  static std::string type_name () { return "CComputeArea"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_field();

  void trigger_elements();
  
private: // data
  
  boost::shared_ptr<Mesh::CScalarFieldView> m_area;
  
  RealMatrix m_coordinates;
    
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CComputeArea_hpp
