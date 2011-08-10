// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Init_hpp
#define CF_RDM_Init_hpp

#include "Math/VectorialFunction.hpp"

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {

namespace Mesh { class CMesh; class Field; }

namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

class RDM_API Init : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<Init> Ptr;
  typedef boost::shared_ptr<Init const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  Init ( const std::string& name );

  /// Virtual destructor
  virtual ~Init() {}

  /// Get the class name
  static std::string type_name () { return "Init"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_function();

private: // data

  boost::weak_ptr<Mesh::Field> m_field;  ///< access to the field to initialize

  Math::VectorialFunction  m_function;    ///< function parser for the math formula

};

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

#endif // CF_RDM_Init_hpp
