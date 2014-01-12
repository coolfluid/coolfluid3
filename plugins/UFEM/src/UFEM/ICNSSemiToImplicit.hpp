// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ICNSSemiToImplicit_hpp
#define cf3_UFEM_ICNSSemiToImplicit_hpp

#include "common/Action.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Initial condition to convert from a solution of the Semi-implicit solver to the implcit one
class UFEM_API ICNSSemiToImplicit : public common::Action
{
public:

  /// Contructor
  /// @param name of the component
  ICNSSemiToImplicit ( const std::string& name );
  
  virtual ~ICNSSemiToImplicit();

  /// Get the class name
  static std::string type_name () { return "ICNSSemiToImplicit"; }
  
  virtual void execute();
};

} // UFEM
} // cf3


#endif // cf3_UFEM_ICNSSemiToImplicit_hpp
