// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CopySolution_hpp
#define CF_RDM_CopySolution_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class Field; }
namespace RDM {

/// Copies the solution to a set of fields
/// @author Tiago Quintino
class RDM_API CopySolution : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CopySolution> Ptr;
  typedef boost::shared_ptr<CopySolution const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CopySolution ( const std::string& name );

  /// Virtual destructor
  virtual ~CopySolution() {}

  /// Get the class name
  static std::string type_name () { return "CopySolution"; }

  /// execute the action
  virtual void execute ();

private: // data

  boost::weak_ptr<Mesh::Field> m_solution;  ///< solution field pointer

};

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_CopySolution_hpp
