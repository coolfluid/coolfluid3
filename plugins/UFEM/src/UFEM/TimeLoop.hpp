// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_TimeLoop_hpp
#define CF_UFEM_TimeLoop_hpp

#include <boost/mpl/assert.hpp>

#include "Common/CActionDirector.hpp"
#include "Common/OptionComponent.hpp"

#include "Solver/CTime.hpp"

#include "LibUFEM.hpp"

namespace CF {

namespace UFEM {

/// Executes an action of the type BaseT in a time loop, retaining the BaseT interface
template<typename BaseT>
class UFEM_API TimeLoop : public BaseT
{
  /// BaseT must be a CAction
  BOOST_MPL_ASSERT(( boost::is_base_of<Common::CAction, BaseT> ));
public: // typedefs

  typedef boost::shared_ptr<TimeLoop> Ptr;
  typedef boost::shared_ptr<TimeLoop const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  TimeLoop ( const std::string& name ) : BaseT(name)
  {
    BaseT::options().add_option(Common::OptionComponent<Solver::CTime>::create("time", &m_time))
        ->set_description("Time tracking component")
        ->set_pretty_name("Time")
        ->mark_basic();
  }

  virtual ~TimeLoop() {}

  /// Get the class name
  static std::string type_name () { return "TimeLoop"; }

  virtual void execute()
  {
    if(m_time.expired())
      throw Common::SetupError(FromHere(), "Error executing TimeLoop " + BaseT::uri().string() + ": Time is invalid");

    Solver::CTime& time = *m_time.lock();
    const Real& t = time.time();
    const Real dt = time.dt();
    Uint iter = time.iter();
    while(t < time.end_time())
    {
      BaseT::execute();
      time.configure_option("iteration", ++iter);
      time.configure_option("time", dt * static_cast<Real>(iter));
    }
  }

private:
  boost::weak_ptr<Solver::CTime> m_time;
};

} // UFEM
} // CF


#endif // CF_UFEM_TimeLoop_hpp
