// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_BCDirichletCons2D_hpp
#define CF_FVM_BCDirichletCons2D_hpp

#include "FVM/BC.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CConnectedFieldView;
}
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API BCDirichletCons2D : public BC
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BCDirichletCons2D> Ptr;
  typedef boost::shared_ptr<BCDirichletCons2D const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BCDirichletCons2D ( const std::string& name );

  /// Virtual destructor
  virtual ~BCDirichletCons2D() {};

  /// Get the class name
  static std::string type_name () { return "BCDirichletCons2D"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();

  void trigger_elements();
  
private: // data
  
  enum {INNER=0,GHOST=1};
  
  Mesh::CConnectedFieldView m_connected_solution;
  
  Real m_rho;
  Real m_u;
  Real m_v;
  Real m_p;
  Real m_gm1;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_BCDirichletCons2D_hpp
