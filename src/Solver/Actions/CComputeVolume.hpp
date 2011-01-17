// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CComputeVolume_hpp
#define CF_Solver_Actions_CComputeVolume_hpp

#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldLink.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  template <typename T> class CTable;
  class CElements;
  class CField2;
}
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CComputeVolume : public CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CComputeVolume> Ptr;
  typedef boost::shared_ptr<CComputeVolume const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CComputeVolume ( const std::string& name );

  /// Virtual destructor
  virtual ~CComputeVolume() {};

  /// Get the class name
  static std::string type_name () { return "CComputeVolume"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_field();

  void trigger_elements();

  Mesh::CField2& volume_field() { return *m_field.lock(); }
  
private: // data

  boost::weak_ptr<Mesh::CField2>       m_field;
  boost::weak_ptr<Mesh::CTable<Uint> > m_index;
    
};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Actions_CComputeVolume_hpp
