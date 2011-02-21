// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_ResidualDistribution_hpp
#define CF_RDM_ResidualDistribution_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CDiscretization.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
  
namespace Mesh { class CField2; }
namespace Solver { namespace Actions {  class CLoop; } }

namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// RDM component class
/// @author Tiago Quintino
/// @author Willem Deconinck
class RDM_API ResidualDistribution : public Solver::CDiscretization {

public: // typedefs

  typedef boost::shared_ptr<ResidualDistribution> Ptr;
  typedef boost::shared_ptr<ResidualDistribution const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ResidualDistribution ( const std::string& name );

  /// Virtual destructor
  virtual ~ResidualDistribution();

  /// Get the class name
  static std::string type_name () { return "ResidualDistribution"; }

  // functions specific to the ResidualDistribution component
  
  /// computes the discrete rhs of the PDE
  virtual void compute_rhs();

  /// @name SIGNALS
  //@{

  /// creates a boundary term
  void create_boundary_term( Common::XmlNode& xml );

  /// creates a domain term
  void create_domain_term( Common::XmlNode& xml );

  //@} END SIGNALS

private: // functions

  void config_mesh();

private: // data

  /// @note still here for compatibility with schemes and bcs

  Common::CLink::Ptr m_solution_field;
  Common::CLink::Ptr m_residual_field;
  Common::CLink::Ptr m_update_coeff_field;

  /// @note new approach

  boost::weak_ptr<CField2> m_solution;
  boost::weak_ptr<CField2> m_residual;
  boost::weak_ptr<CField2> m_update_coeff;

  /// action to compute the boundary face terms
  Common::CAction::Ptr m_compute_boundary_face_terms;

  /// action to compute the domain cell terms
  Common::CAction::Ptr m_compute_volume_cell_terms;
};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_ResidualDistribution_hpp
