// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CModel_hpp
#define CF_Solver_CModel_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Mesh { class CDomain; }
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// CModel is the top most component on a simulation structure
/// CModel now stores:
/// - Physical model
/// - Iterative solver
/// - Discretization
/// @author Martin Vymazal
class Solver_API CModel : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CModel> Ptr;
  typedef boost::shared_ptr<CModel const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CModel ( const std::string& name );

  /// Virtual destructor
  virtual ~CModel();

  /// Get the class name
  static std::string type_name () { return "CModel"; }

  /// creates a domain in this model
  virtual boost::shared_ptr<Mesh::CDomain> create_domain( const std::string& name );

  /// Simulates this model
  virtual void simulate() = 0;

  /// @name SIGNALS
  //@{

  /// Signature of create domain signal @see signal_create_domain
  void signature_create_domain ( Common::XmlNode& node );
  /// Signal to create a domain and load a mesh into it
  void signal_create_domain ( Common::XmlNode& node );

  /// Signal to start simulating
  void signal_simulate ( Common::XmlNode& node );

  //@} END SIGNALS
  

protected:
  
  /// path to working directory
  Common::URI m_working_dir;

  /// path to results directory
  Common::URI m_results_dir;

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CModel_hpp
