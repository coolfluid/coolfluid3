#include "Common/CF.hh"

#ifdef CF_HAVE_MPI
#  include "Common/MPI/PEInterfaceMPI.hh"
#else
#error Do not have MPI!
 //#  include "Common/SERIAL/PEInterfaceSERIAL.hh"
#endif // CF_HAVE_MPI

#include "Common/PE.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

//////////////////////////////////////////////////////////////////////////////

// initialize the static data
PEInterface<>      * PE::m_curr_PE = CFNULL;
bool                 PE::m_is_init = false;
char               * PE::m_command_workers = CFNULL;
WorkerStatus::Type   PE::m_current_status = WorkerStatus::NOT_RUNNING;

//////////////////////////////////////////////////////////////////////////////

bool PE::IsInitialised ()
{
  return m_is_init;
}

//////////////////////////////////////////////////////////////////////////////

void PE::InitPE (int * argc, char *** args)
{
    cf_assert (m_curr_PE == CFNULL);

    m_command_workers = new char[strlen(*args[0]) + 1];
    strcpy(m_command_workers, *args[0]);

    m_curr_PE = new PEInterface<>(argc, args);
    cf_assert (m_curr_PE != CFNULL);
    m_is_init = true;
}

//////////////////////////////////////////////////////////////////////////////

PEInterface<> & PE::GetPE ()
{
  cf_assert(m_is_init);
  cf_assert (m_curr_PE != CFNULL);
  return *m_curr_PE;
}

//////////////////////////////////////////////////////////////////////////////

void PE::DonePE ()
{
  cf_assert(m_curr_PE != CFNULL);

  // must be first to make sure all destructors dependent of PE acknowledge MPI is down
  m_is_init = false;
  delete_ptr(m_curr_PE);
  delete_ptr_array(m_command_workers);
}

//////////////////////////////////////////////////////////////////////////////

MPI_Comm PE::spawn(unsigned int count, const char * hosts)
{
MPI_Info info;
MPI_Comm comm;
char command[] = "/nobackup/st/gasper/coolfluid/x86_64/optim/plugins/ClientServer/server/app_server";
int myRank = m_curr_PE->GetRank();

MPI_Info_create(&info);

if(hosts != CFNULL && strlen(hosts) != 0)
{
  char hostKey[] = "host";
  char * hostsNonConst;

  hostsNonConst = new char[strlen(hosts) + 1];
  strcpy(hostsNonConst, hosts);

  CFLogInfo("Spawning " << count << " worker(s) on the following host(s): "
    << hosts << ".\n");
  MPI_Info_set(info, hostKey, hostsNonConst);

  delete_ptr_array(hostsNonConst);
}
else
{
  // not giving the "host" value should be sufficient (localhost is the default
  // when no value is provided)
  // this does not work here since we pass a hostfile : MPI uses this file
  // to determine which hosts to use if no "host" value is provided...
  // setting "host" explicitly to "localhost" prevents this unwanted behaviour
  MPI_Info_set(info, "host", "localhost");
  CFLogInfo("Spawning " << count << " worker(s) on local host.\n");
}

MPI_Comm_spawn(command,        // command to run
                MPI_ARGV_NULL,  // arguments to the command
                count,          // number of processes
                info,           // infos
                myRank, // manager (root) rank
                m_curr_PE->GetCommunicator(),
                &comm,
                MPI_ERRCODES_IGNORE);

return comm;
}

//////////////////////////////////////////////////////////////////////////////

void PE::setCurrentStatus(WorkerStatus::Type status)
{
cf_assert ( WorkerStatus::Convert::is_valid(status) );
m_current_status = status;
}

//////////////////////////////////////////////////////////////////////////////

WorkerStatus::Type PE::getCurrentStatus()
{
return m_current_status;
}

//////////////////////////////////////////////////////////////////////////////

  } // Common

} // CF

