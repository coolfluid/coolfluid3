// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Cleanup_hpp
#define CF_RDM_Cleanup_hpp

#include "Solver/Action.hpp"

#include "RDM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class CField; }
namespace RDM {
namespace Core {

class RDM_Core_API Cleanup : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<Cleanup> Ptr;
  typedef boost::shared_ptr<Cleanup const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  Cleanup ( const std::string& name );

  /// Virtual destructor
  virtual ~Cleanup() {}

  /// Get the class name
  static std::string type_name () { return "Cleanup"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_fields();

private: // data

  std::vector< boost::weak_ptr<Mesh::CField> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF

#endif // CF_RDM_Cleanup_hpp
