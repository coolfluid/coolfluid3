#ifndef CF_Common_CMethod_hpp
#define CF_Common_CMethod_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CMethod : public Component {

  public: // typedefs

    /// provider
    typedef Common::ConcreteProvider < CMethod,1 > PROVIDER;
    /// pointer to this type
    typedef boost::shared_ptr<CMethod> Ptr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CMethod ( const CName& name );

    /// Virtual destructor
    virtual ~CMethod();

    /// Get the class name
    static std::string getClassName () { return "CMethod"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options ) {}

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CMethod_hpp
