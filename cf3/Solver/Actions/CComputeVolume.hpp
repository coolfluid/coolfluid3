// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Actions_CComputeVolume_hpp
#define cf3_Solver_Actions_CComputeVolume_hpp

#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
  template <typename T> class CTable;
  class CElements;
  class Field;
  class CScalarFieldView;
  class CSpace;
}
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CComputeVolume : public CLoopOperation {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CComputeVolume> Ptr;
  typedef boost::shared_ptr<CComputeVolume const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CComputeVolume ( const std::string& name );

  /// Virtual destructor
  virtual ~CComputeVolume() {}

  /// Get the class name
  static std::string type_name () { return "CComputeVolume"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_field();

  void trigger_elements();

private: // data

  boost::weak_ptr<Mesh::Field> m_volume;
  boost::weak_ptr<Mesh::CSpace> m_volume_field_space;

  RealMatrix m_coordinates;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Solver_Actions_CComputeVolume_hpp
