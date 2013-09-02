// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_WriteRestartManager_hpp
#define cf3_UFEM_WriteRestartManager_hpp

#include "solver/actions/WriteRestartFile.hpp"
#include "solver/actions/TimeSeriesWriter.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
namespace UFEM {

/// Helper class to manage the writing of restart files
class UFEM_API WriteRestartManager : public solver::actions::TimeSeriesWriter
{
public: // functions

  /// Contructor
  /// @param name of the component
  WriteRestartManager ( const std::string& name );
  virtual ~WriteRestartManager();
  static std::string type_name () { return "WriteRestartManager"; }
  
private:
  Handle<solver::actions::WriteRestartFile> m_write_restart;
  
  void trigger_setup();
};

} // UFEM
} // cf3


#endif // cf3_UFEM_WriteRestartManager_hpp
