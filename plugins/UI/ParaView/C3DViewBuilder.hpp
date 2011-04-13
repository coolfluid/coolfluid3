#ifndef CF_UI_ParaView_C3DVIEWBUILDER_HPP
#define CF_UI_ParaView_C3DVIEWBUILDER_HPP

#include "Common/Component.hpp"

#include "UI/ParaView/C3DView.hpp"

#include "Solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace ParaView {

////////////////////////////////////////////////////////////////////////////////

class /*ParaView_API*/ C3DViewBuilder :
    public Common::Component
{
public: // typedefs

  typedef boost::shared_ptr<C3DViewBuilder> Ptr;
  typedef boost::shared_ptr<C3DViewBuilder const> ConstPtr;

public:

  C3DViewBuilder(const std::string & name);

  static std::string type_name() { return "C3DViewBuilder"; }

  /// @name SIGNALS
  //@{

  void signal_create_3dview( Common::SignalArgs & args);

  void signature_create_3dview( Common::SignalArgs & args);

  //@} END SIGNALS

private: // data

  //process id
  //port
  //host
  //path

}; // C3DViewBuilder

////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_ParaView_C3DVIEWBUILDER_HPP
