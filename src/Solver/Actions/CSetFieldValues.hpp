// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CSetFieldValues_hpp
#define CF_Solver_Actions_CSetFieldValues_hpp

#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  template <typename T> class CTable;
  class CElements;
  class CField;
}
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CSetFieldValues : public CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CSetFieldValues> Ptr;
  typedef boost::shared_ptr<CSetFieldValues const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSetFieldValues ( const std::string& name );

  /// Virtual destructor
  virtual ~CSetFieldValues() {}

  /// Get the class name
  static std::string type_name () { return "CSetFieldValues"; }

  /// execute the action
  virtual void execute ();

private: // data

  boost::weak_ptr<Mesh::CField> m_field;

  void config_field();
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CSetFieldValues_hpp
