#include <boost/assign/list_of.hpp> // for map_list_of

#include "Common/WorkerStatus.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace COOLFluiD::Common;

 //////////////////////////////////////////////////////////////////////////////

WorkerStatus::Convert::FwdMap_t WorkerStatus::Convert::all_fwd = boost::assign::map_list_of
   ( WorkerStatus::INVALID,     "INVALID" )
   ( WorkerStatus::STARTING,    "Starting")
   ( WorkerStatus::CONFIGURING, "Configuring")
   ( WorkerStatus::EXITING,     "Exiting")
   ( WorkerStatus::RUNNING,     "Running")
   ( WorkerStatus::NOT_RUNNING, "Not Running")
   ( WorkerStatus::WAITING,     "Waiting")
   ( WorkerStatus::IDLE,        "Idle");

 WorkerStatus::Convert::BwdMap_t WorkerStatus::Convert::all_rev = boost::assign::map_list_of
   ("INVALID",     WorkerStatus::INVALID)
   ("Starting",    WorkerStatus::STARTING)
   ("Configuring", WorkerStatus::CONFIGURING)
   ("Exiting",     WorkerStatus::EXITING)
   ("Running",     WorkerStatus::RUNNING)
   ("Not Running", WorkerStatus::NOT_RUNNING)
   ("Waiting",     WorkerStatus::WAITING)
   ("Idle",        WorkerStatus::IDLE);

 //////////////////////////////////////////////////////////////////////////////

 std::ostream& operator<< ( std::ostream& os, const WorkerStatus::Type& in )
 {
  os << WorkerStatus::Convert::to_str(in);
  return os;
 }

 std::istream& operator>> (std::istream& is, WorkerStatus::Type& in )
 {
  std::string tmp;
  is >> tmp;
  in = WorkerStatus::Convert::to_enum(tmp);
  return is;
 }


//////////////////////////////////////////////////////////////////////////////
