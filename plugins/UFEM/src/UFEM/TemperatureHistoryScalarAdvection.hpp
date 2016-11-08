// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_TemperatureHistoryScalarAdvection_hpp
#define cf3_UFEM_TemperatureHistoryScalarAdvection_hpp

#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEM.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

class UFEM_API TemperatureHistoryScalarAdvection : public solver::actions::Proto::ProtoAction
{
public:

  /// Contructor
  /// @param name of the component
  TemperatureHistoryScalarAdvection ( const std::string& name );

  virtual ~TemperatureHistoryScalarAdvection();

  /// Get the class name
  static std::string type_name () { return "TemperatureHistoryScalarAdvection"; }

  /// Execute
  virtual void execute();

  private:

  std::ofstream convergence_history;
  Real m_min_error;
  Real m_max_error;

};

} // UFEM
} // cf3


#endif // cf3_UFEM_TemperatureHistoryScalarAdvection_hpp
