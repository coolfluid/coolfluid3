// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_IterativeSolver_hpp
#define CF_SFDM_IterativeSolver_hpp

#include "Common/CAction.hpp"

#include "SFDM/LibSFDM.hpp"

namespace CF {
namespace Common { class CActionDirector; }
namespace SFDM {


/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API IterativeSolver : public Common::CAction {

public: // typedefs

  typedef boost::shared_ptr<IterativeSolver> Ptr;
  typedef boost::shared_ptr<IterativeSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  IterativeSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~IterativeSolver() {}

  /// Get the class name
  static std::string type_name () { return "IterativeSolver"; }

  /// execute the action
  virtual void execute ();

  Common::CActionDirector& pre_update()    { return *m_pre_update; }
  Common::CActionDirector& update()        { return *m_update; }
  Common::CActionDirector& post_update()   { return *m_post_update; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// @returns true if any of the stop criteria is achieved
  virtual bool stop_condition();
  /// raises the event when iteration done
  void raise_iteration_done();

private: // data

  /// set of actions called every iteration before non-linear solve
  boost::shared_ptr<Common::CActionDirector> m_pre_update;
  /// set of actions called every iteration to update the solution
  boost::shared_ptr<Common::CActionDirector> m_update;
  /// set of actions called every iteration after non-linear solve
  boost::shared_ptr<Common::CActionDirector> m_post_update;

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // CF

#endif // CF_SFDM_IterativeSolver_hpp
