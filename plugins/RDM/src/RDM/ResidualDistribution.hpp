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
  
  namespace Actions { class CLoop; }

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
  
  virtual void compute_rhs();
  
  void create_bc( Common::XmlNode& xml );

private: // functions
  
  void trigger_Regions();
  
private: // data

  boost::shared_ptr<Actions::CLoop> m_elem_loop;
};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_ResidualDistribution_hpp
