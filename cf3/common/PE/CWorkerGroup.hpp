// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_PE_CWorkerGroup_hpp
#define cf3_common_PE_CWorkerGroup_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CommonAPI.hpp"
#include "common/Component.hpp"

#include "common/PE/types.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace PE {

////////////////////////////////////////////////////////////////////////////////

class Common_API CWorkerGroup : public Component
{
public: // typedefs

  typedef boost::shared_ptr<CWorkerGroup> Ptr;
  typedef boost::shared_ptr<const CWorkerGroup> ConstPtr;

public:

  CWorkerGroup( const std::string & name );

  /// Destructor.
  virtual ~CWorkerGroup();

  /// Returns the class name.
  static std::string type_name() { return "CWorkerGroup"; }

  void set_communicator( Communicator comm );

  Communicator communicator() const;

  int nbworkers () const;

  /// @name SIGNALS
  //@{

  void signal_solve( common::SignalArgs & args);

  //@} END SIGNALS

private:

  Communicator m_comm;

};

////////////////////////////////////////////////////////////////////////////////

} // namespace PE
} // namespace common
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_PE_CWorkerGroup_hpp
