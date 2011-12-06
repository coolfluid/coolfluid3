// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_TimeLoop_hpp
#define cf3_UFEM_TimeLoop_hpp

#include <boost/scoped_ptr.hpp>

#include "common/ActionDirector.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// Executes the added actions in a time loop.
class UFEM_API TimeLoop : public common::ActionDirector
{
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  TimeLoop ( const std::string& name );

  virtual ~TimeLoop();

  /// Get the class name
  static std::string type_name () { return "TimeLoop"; }

  virtual void execute();

private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_TimeLoop_hpp
