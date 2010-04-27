#ifndef CF_server_RecievingProcess_h
#define CF_server_RecievingProcess_h
////////////////////////////////////////////////////////////////////////////

#include <mpi.h>

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {
      
////////////////////////////////////////////////////////////////////////////
      
      
  //   struct ReceivedFrameInfo
  //   {
  //    CF::Common::BuilderParserFrameInfo frame;
  
  //   };
  
  /// @todo this class should be removed
  
  class MPIReceiver
  {
  public:
    
    static void receive(CF::Common::BuilderParserFrameInfo & frame,
                        MPI::Intercomm comm,
                        const CF::Common::BuilderParserRules & rules);
  }; // class ReceivingProcess
  
////////////////////////////////////////////////////////////////////////////
  
} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_server_RecievingProcess_h
