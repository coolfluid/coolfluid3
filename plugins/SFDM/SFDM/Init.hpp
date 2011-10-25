// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Init_hpp
#define cf3_SFDM_Init_hpp

#include "math/VectorialFunction.hpp"

#include "solver/Action.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {

namespace physics { class Variables; }
namespace mesh    { class Mesh; class Field; }

namespace SFDM {

////////////////////////////////////////////////////////////////////////////////////////////

class SFDM_API Init : public cf3::solver::Action {

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

  boost::weak_ptr<mesh::Field> m_field;  ///< access to the field to initialize
  boost::weak_ptr<physics::Variables> m_input_vars;  ///< access to the input variables
  math::VectorialFunction  m_function;    ///< function parser for the math formula

};

////////////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

#endif // cf3_SFDM_Init_hpp
