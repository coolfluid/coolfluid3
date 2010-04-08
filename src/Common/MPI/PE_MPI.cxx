#include "Common/MPI/PE_MPI.hh"
#include "Common/MPI/MPIInitObject.hh"
#include "Common/MPI/HelperFuncs.hh"

#include "Common/Log.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace MPI  {

//////////////////////////////////////////////////////////////////////////////

static void ThrowMPI (MPI_Comm * Comm, int * Error, ...)
{
  ThrowErrorHere (*Error, FromHere());
}

//////////////////////////////////////////////////////////////////////////////

PE_MPI::PE_MPI (int * argc, char *** args, MPI_Comm UsedCom) :
    m_comm(UsedCom),
    /*DataTypeHandler (UsedCom),*/
    m_init_ok(false),
    m_stop_called(false),
    m_command_workers (CFNULL)
{
    int Ret = MPI_Init (argc, args);
    if (Ret != MPI_SUCCESS) throw std::string("MPI_Init failed!");
    m_init_ok = true;

    m_command_workers = new char[strlen(*args[0]) + 1];
    strcpy(m_command_workers, *args[0]);

    CheckMPIStatus(MPI_Errhandler_create (ThrowMPI, &m_error_handler));
    CheckMPIStatus(MPI_Errhandler_set (m_comm, m_error_handler));

  // DataTypeHandler.InitTypes ();

    CallInitFunctions ();
}

//////////////////////////////////////////////////////////////////////////////

PE_MPI::~PE_MPI ()
{
  CallDoneFunctions ();

  /* DataTypeHandler.DoneTypes (); */

  delete_ptr_array(m_command_workers);

  MPI_Finalize ();
  m_init_ok=false;
}

//////////////////////////////////////////////////////////////////////////////

std::string PE_MPI::getModelName () const
{
  return "MPI";
}

//////////////////////////////////////////////////////////////////////////////

MPI_Comm PE_MPI::getCommunicator () const
{
  return MPI_COMM_WORLD;
}

//////////////////////////////////////////////////////////////////////////////

Uint PE_MPI::get_procesor_count () const
{
  int size;
  MPI_Comm_size (m_comm, &size);
  return static_cast<Uint>(size);
}

//////////////////////////////////////////////////////////////////////////////

Uint PE_MPI::get_rank () const
{
    static int Rank = -1;
    if (Rank != -1)
  return Rank;

    MPI_Comm_rank (m_comm, &Rank);
    return static_cast<unsigned int>(Rank);
}

//////////////////////////////////////////////////////////////////////////////

bool PE_MPI::is_parallel () const
{
    return ( get_procesor_count() > 1 );
}

//////////////////////////////////////////////////////////////////////////////

void PE_MPI::CallInitFunctions ()
{
    CFLogDebug( "PE_MPI::CallInitFunctions()\n");
    cf_assert (!m_stop_called);
    InitContainerType::iterator Cur = m_init_list.begin();
    while (Cur != m_init_list.end())
    {
	CallInitFunction (*Cur);
	++Cur;
    }
}

//////////////////////////////////////////////////////////////////////////////

void PE_MPI::CallDoneFunctions ()
{
    CFLogDebug( "PE_MPI::CallDoneFunctions ()\n");
    InitContainerType::iterator Cur = m_init_list.begin();
    while (Cur != m_init_list.end())
    {
	CallDoneFunction (*Cur);
	++Cur;
    }
    m_stop_called = true;
    // Clear the list
    m_init_list.clear ();
}

//////////////////////////////////////////////////////////////////////////////

void PE_MPI::CallInitFunction (MPIInitObject * NewObj) const
{
    CFLogDebug( "Calling MPI_Init on " << NewObj << "\n");
    NewObj->MPI_Init (getCommunicator());
}

//////////////////////////////////////////////////////////////////////////////

void PE_MPI::CallDoneFunction (MPIInitObject * NewObj) const
{
    CFLogDebug( "Calling MPI_Done on " << NewObj << "\n");
    NewObj->MPI_Done ();
}

//////////////////////////////////////////////////////////////////////////////

void PE_MPI::registInitObj (MPIInitObject * NewObj)
{
    cf_assert (!m_stop_called);

    m_init_list.push_back (NewObj);

    if (m_init_ok)
    {
  // Initialization is already started, so we can call init immediately
  CallInitFunction (m_init_list.back());
    }
}

//////////////////////////////////////////////////////////////////////////////

MPI_Comm PE_MPI::spawn(unsigned int count, const char * hosts)
{
  MPI_Info info;
  MPI_Comm comm;
  char command[] = "/nobackup/st/gasper/coolfluid/x86_64/optim/plugins/ClientServer/server/app_server";
  int myRank = get_rank();

  MPI_Info_create(&info);

  if(hosts != CFNULL && strlen(hosts) != 0)
  {
    char host_str [] = "host";
    char * hosts_non_const;

    hosts_non_const = new char[strlen(hosts) + 1];
    strcpy(hosts_non_const, hosts);

    CFLogInfo("Spawning " << count << " worker(s) on the following host(s): " << hosts_non_const << ".\n");
    MPI_Info_set(info, host_str, hosts_non_const);

    delete_ptr_array(hosts_non_const);
  }
  else
  {
    // not giving the "host" value should be sufficient (localhost is the default
    // when no value is provided)
    // this does not work here since we pass a hostfile : MPI uses this file
    // to determine which hosts to use if no "host" value is provided...
    // setting "host" explicitly to "localhost" prevents this unwanted behaviour
    char host_str [] = "host";
    char localhost_str [] = "localhost";
    MPI_Info_set(info, host_str, localhost_str);
    CFLogInfo("Spawning " << count << " worker(s) on local host.\n");
  }

  MPI_Comm_spawn(command,        // command to run
                  MPI_ARGV_NULL,  // arguments to the command
                  count,          // number of processes
                  info,           // infos
                  myRank, // manager (root) rank
                  getCommunicator(),
                  &comm,
                  MPI_ERRCODES_IGNORE);

  return comm;
}

//////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF
