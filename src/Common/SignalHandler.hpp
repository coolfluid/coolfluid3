#ifndef CF_Common_SignalHandler_hpp
#define CF_Common_SignalHandler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/OwnedObject.hpp"

#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// This class is an interface for handlers of signals from the operating system
  /// @author Tiago Quintino
  class Common_API SignalHandler : public boost::noncopyable {

  public: // methods

    /// Constructor is private to allow only the friend classes to build it
    SignalHandler();

    /// Default destructor is private to allow only the friend classes to destroy it
    virtual ~SignalHandler();

    /// Gets the Class name
    static std::string getClassName() { return "SignalHandler"; }

  protected: // methods

  private: // data

  }; // SignalHandler

  ////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // COOFluiD_Common_SignalHandler_hpp
