// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_TimeLoop_hpp
#define CF_UFEM_TimeLoop_hpp

#include <boost/scoped_ptr.hpp>

#include "Common/CActionDirector.hpp"

#include "LibUFEM.hpp"

namespace CF {

namespace UFEM {

/// Executes the added actions in a time loop.
class UFEM_API TimeLoop : public Common::CActionDirector
{
public: // typedefs

  typedef boost::shared_ptr<TimeLoop> Ptr;
  typedef boost::shared_ptr<TimeLoop const> ConstPtr;

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
} // CF


#endif // CF_UFEM_TimeLoop_hpp
