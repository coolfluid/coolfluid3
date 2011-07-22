// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Reset_hpp
#define CF_RDM_Reset_hpp

#include "Solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh { class CField; }
namespace RDM {


class RDM_API Reset : public CF::Solver::Action {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<Reset> Ptr;
  typedef boost::shared_ptr<Reset const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  Reset ( const std::string& name );

  /// Virtual destructor
  virtual ~Reset() {}

  /// Get the class name
  static std::string type_name () { return "Reset"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_fields();
  void config_field_tags();

private: // data

  std::vector< boost::weak_ptr<Mesh::CField> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF

#endif // CF_RDM_Reset_hpp
