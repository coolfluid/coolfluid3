// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_SFDWizard_hpp
#define CF_SFDM_SFDWizard_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CWizard.hpp"
#include "Solver/Action.hpp"
#include "SFDM/LibSFDM.hpp"
#include "Math/MathConsts.hpp"
////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common { class CLink; }
namespace Solver { class CModelUnsteady; }
namespace SFDM {

//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that creates a space in the mesh
/// with SFDM shape functions, and a configurable order of polynomial.
/// Default polynomial order = 0.
/// that returns information about the mesh
/// @author Willem Deconinck
class SFDM_API SFDWizard : public Solver::CWizard
{
public: // typedefs

    typedef boost::shared_ptr<SFDWizard> Ptr;
    typedef boost::shared_ptr<SFDWizard const> ConstPtr;

public: // functions

  /// constructor
  SFDWizard( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "SFDWizard"; }

  void create_simulation();

  void prepare_simulation();

  void start_simulation(const Real& end_time, const Real& time_step=Math::MathConsts::Real_max());

  void initialize_solution(const std::vector<std::string>& functions);

  /// @name SIGNALS
  //@{

  void signal_create_simulation( Common::SignalArgs& xml );
  void signature_create_simulation( Common::SignalArgs& xml );

  void signal_prepare_simulation( Common::SignalArgs& xml );
  void signature_prepare_simulation( Common::SignalArgs& xml );

  void signal_initialize_solution( Common::SignalArgs& xml );
  void signature_initialize_solution( Common::SignalArgs& xml );

  void signal_start_simulation( Common::SignalArgs& xml );
  void signature_start_simulation( Common::SignalArgs& xml );

  //@} END SIGNALS

  Solver::CModelUnsteady& model();

private: // functions

  void build_solve();
  void build_setup();

private: // data
  boost::shared_ptr<Common::CLink> m_model_link;

}; // end SFDWizard

////////////////////////////////////////////////////////////////////////////////

class SFDM_API SFDSetup : public Solver::Action
{
public: // typedefs

    typedef boost::shared_ptr<SFDSetup> Ptr;
    typedef boost::shared_ptr<SFDSetup const> ConstPtr;

public: // functions

  /// constructor
  SFDSetup( const std::string& name ) : Solver::Action(name) {}

  /// Gets the Class name
  static std::string type_name() { return "SFDSetup"; }

  virtual void execute();

};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_SFDWizard_hpp
