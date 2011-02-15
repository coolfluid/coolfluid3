// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_BcDirichlet_hpp
#define CF_Solver_Actions_BcDirichlet_hpp

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace Mesh
{
  class CField2;
  class CFieldView;
}

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API BcDirichlet : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BcDirichlet> Ptr;
  typedef boost::shared_ptr<BcDirichlet const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BcDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~BcDirichlet() {};

  /// Get the class name
  static std::string type_name () { return "BcDirichlet"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void trigger_elements();
  void config_field();

private: // data

  boost::shared_ptr<Mesh::CFieldView> m_field_view;
  boost::weak_ptr<Mesh::CField2> m_field;

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_BcDirichlet_hpp
