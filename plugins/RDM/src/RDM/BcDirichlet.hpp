// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_BcDirichlet_hpp
#define cf3_RDM_BcDirichlet_hpp

#include "Math/VectorialFunction.hpp"

#include "RDM/BoundaryTerm.hpp"

namespace cf3 {

namespace Mesh { class CMesh; class Field; }

namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

class RDM_API BcDirichlet : public RDM::BoundaryTerm {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BcDirichlet> Ptr;
  typedef boost::shared_ptr<BcDirichlet const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BcDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~BcDirichlet() {}

  /// Get the class name
  static std::string type_name () { return "BcDirichlet"; }

  /// execute the action
  virtual void execute ();

  virtual bool is_weak() const { return false; }

private: // helper functions

  void config_function();

private: // data

  /// access to the solution field on the mesh
  boost::weak_ptr<Mesh::Field> m_solution;
  /// function parser for the math formula of the dirichlet condition
  Math::VectorialFunction  m_function;

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_BcDirichlet_hpp
