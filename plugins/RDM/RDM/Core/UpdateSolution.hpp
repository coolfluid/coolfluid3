// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_UpdateSolution_hpp
#define CF_RDM_UpdateSolution_hpp

#include "RDM/Core/Action.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class CField; }
namespace RDM {

class RDM_CORE_API UpdateSolution : public Solver::Action
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<UpdateSolution> Ptr;
  typedef boost::shared_ptr<UpdateSolution const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  UpdateSolution ( const std::string& name );

  /// Virtual destructor
  virtual ~UpdateSolution() {};

  /// Get the class name
  static std::string type_name () { return "UpdateSolution"; }
  
  /// execute the action
  virtual void execute ();

private: // data

  /// solution field pointer
  boost::weak_ptr<Mesh::CField> m_solution;
  /// residual field pointer
  boost::weak_ptr<Mesh::CField> m_residual;
  /// wave_speed field pointer
  boost::weak_ptr<Mesh::CField> m_wave_speed;

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_UpdateSolution_hpp
