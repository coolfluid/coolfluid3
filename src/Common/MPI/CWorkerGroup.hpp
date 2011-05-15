// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_mpi_CWorkerGroup_hpp
#define CF_Common_mpi_CWorkerGroup_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"
#include "Common/Component.hpp"

#include "Common/MPI/types.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace mpi {

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

private:

  Communicator m_comm;

};

////////////////////////////////////////////////////////////////////////////////

} // namespace mpi
} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_mpi_CWorkerGroup_hpp
