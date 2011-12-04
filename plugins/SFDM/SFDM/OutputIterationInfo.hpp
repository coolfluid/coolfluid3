// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_OutputIterationInfo_hpp
#define CF_SFDM_OutputIterationInfo_hpp

#include "solver/Action.hpp"
#include "SFDM/LibSFDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class CField; }
namespace solver { class CTime;  }
namespace SFDM {

class SFDM_API OutputIterationInfo : public solver::Action
{
public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  OutputIterationInfo ( const std::string& name );

  /// Virtual destructor
  virtual ~OutputIterationInfo() {};

  /// Get the class name
  static std::string type_name () { return "OutputIterationInfo"; }

  /// execute the action
  virtual void execute ();

private: // data

  Handle<mesh::CField> m_residual;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_OutputIterationInfo_hpp
