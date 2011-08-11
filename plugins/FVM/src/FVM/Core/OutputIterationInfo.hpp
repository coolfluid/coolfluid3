// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_OutputIterationInfo_hpp
#define CF_FVM_Core_OutputIterationInfo_hpp

#include "Common/CAction.hpp"
#include "FVM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class CField; }
namespace Solver { class CTime;   }
namespace FVM {
namespace Core {

class FVM_Core_API OutputIterationInfo : public Common::CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<OutputIterationInfo> Ptr;
  typedef boost::shared_ptr<OutputIterationInfo const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  OutputIterationInfo ( const std::string& name );

  /// Virtual destructor
  virtual ~OutputIterationInfo() {}

  /// Get the class name
  static std::string type_name () { return "OutputIterationInfo"; }
  
  /// execute the action
  virtual void execute ();
  
private: // data

  boost::weak_ptr<Solver::CTime> m_time;
  boost::weak_ptr<Mesh::CField> m_residual;  
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_OutputIterationInfo_hpp
