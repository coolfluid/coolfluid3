// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_BCFeedback_hpp
#define cf3_UFEM_BCFeedback_hpp


#include "solver/Action.hpp"
#include "solver/actions/Proto/DirichletBC.hpp"

#include "LibUFEM.hpp"

#include "LSSAction.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
  namespace mesh { class Region; }
namespace UFEM {

/// Feed back outlet values to the inlet, multiplied by a factor
class UFEM_API BCFeedback : public solver::Action
{
public:

  BCFeedback ( const std::string& name );
  
  virtual ~BCFeedback();

  static std::string type_name () { return "BCFeedback"; }

  virtual void execute() override;

private:

  cf3::solver::actions::Proto::DirichletBC m_dirichlet;
  Real m_factor = 1.0;

  std::string m_field_tag = "some_tag";
  std::string m_variable_name = "SomeVar";

  Real m_integral_result;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_BCFeedback_hpp
