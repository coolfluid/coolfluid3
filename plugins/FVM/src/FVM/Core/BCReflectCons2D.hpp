// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_BCReflectCons2D_hpp
#define CF_FVM_Core_BCReflectCons2D_hpp

#include "FVM/Core/BC.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CConnectedFieldView;
  class FieldView;
}
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_Core_API BCReflectCons2D : public BC
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BCReflectCons2D> Ptr;
  typedef boost::shared_ptr<BCReflectCons2D const> ConstPtr;

public: // functions
  
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  /// Contructor
  /// @param name of the component
  BCReflectCons2D ( const std::string& name );

  /// Virtual destructor
  virtual ~BCReflectCons2D() {}

  /// Get the class name
  static std::string type_name () { return "BCReflectCons2D"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();
  void config_normal();

  void trigger_elements();
  
private: // data
  
  enum {INNER=0,GHOST=1};
  
  RealVector2 normal;
  RealVector2 U;
  RealVector2 U_n;
  RealVector2 U_t;
  
  Mesh::CConnectedFieldView m_connected_solution;
  Mesh::FieldView          m_face_normal;

};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_BCReflectCons2D_hpp
