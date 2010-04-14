#ifndef CF_Common_ObjectProvider_hh
#define CF_Common_ObjectProvider_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/Log.hpp"

#include "Common/SelfRegistPtr.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a concrete templated provider to create
/// whatever kind of polymorphic objects. By default it takes two
/// template parameters and it builds a non configurable object
/// whose create() method accepts no arguments.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class MODULE, int NBARGS = 0>
class ObjectProvider : public BASE::PROVIDER {
public:

  /// Constructor
  explicit ObjectProvider(const std::string& name) : BASE::PROVIDER(name)
  {
    CFtrace << "Creating provider [" << name << "] of type [" << BASE::getClassName() << "]\n";
    //Common::ModuleRegister<MODULE>::getInstance().getSelfRegistry().regist(this);
  }

  /// Destructor
  /// @todo unregistration is currently not working
  ~ObjectProvider()
  {
    // Common::ModuleRegister<MODULE>::getInstance().getSelfRegistry().unregist(this);
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @return SelfRegistPtr olding the created object
  Common::SelfRegistPtr<BASE> create()
  {
    return Common::SelfRegistPtr<BASE>(new CONCRETE(), this);
  }

  /// Free an instance created by this factory
  /// @param ptr pointer to be freed
  void freeInstance ( void* ptr )
  {
    cf_assert(ptr != CFNULL);
    CONCRETE* obj = reinterpret_cast<CONCRETE*>(ptr);

    cf_assert(obj != CFNULL);
    delete_ptr<CONCRETE>( obj );
  }

}; // end of class ObjectProvider

////////////////////////////////////////////////////////////////////////////////

/// This class represents a concrete templated provider to create
/// whatever kind of polymorphic objects. It takes four
/// template parameters and it builds a non configurable object
/// whose create() method accepts one arguments.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class MODULE>
class ObjectProvider<CONCRETE,BASE,MODULE, 1> : public BASE::PROVIDER {
public:

  /// Constructor
  explicit ObjectProvider(const std::string& name) : BASE::PROVIDER(name)
  {
    CFLogDebugVerbose (  "Creating provider \'" << name << "\' of type \'" << BASE::getClassName() << "\'\n" );
    MODULE::getInstance().getSelfRegistry().regist(this);
  }

  /// Destructor
  /// @todo unregistration is currently not working
  ~ObjectProvider()
  {
    // Common::ModuleRegister<MODULE>::getInstance().getSelfRegistry().unregist(this);
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @return SelfRegistPtr olding the created object
  Common::SelfRegistPtr<BASE> create(typename BASE::ARG1 arg)
  {
    return Common::SelfRegistPtr<BASE>(new CONCRETE(arg), this);
  }

  /// Free an instance created by this factory
  /// @param ptr pointer to be freed
  void freeInstance ( void* ptr )
  {
    cf_assert(ptr != CFNULL);
    CONCRETE* obj = reinterpret_cast<CONCRETE*>(ptr);

    cf_assert(obj != CFNULL);
    delete_ptr<CONCRETE>( obj );
  }

}; // end of class ObjectProvider

////////////////////////////////////////////////////////////////////////////////

/// This class represents a concrete templated provider to create
/// whatever kind of polymorphic objects. It takes four
/// template parameters and it builds a non configurable object
/// whose create() method accepts one arguments.
/// @author Andrea Lani
/// @author Tiago Quintino
template <class CONCRETE, class BASE, class MODULE>
class ObjectProvider<CONCRETE,BASE,MODULE, 2> : public BASE::PROVIDER {
public:

  /// Constructor
  explicit ObjectProvider(const std::string& name) :
    BASE::PROVIDER(name)
  {
    CFLogDebugVerbose (  "Creating provider \'" << name << "\' of type \'" << BASE::getClassName() << "\'\n" );
    MODULE::getInstance().getSelfRegistry().regist(this);
  }

  /// Destructor
  /// @todo unregistration is currently not working
  ~ObjectProvider()
  {
    // Common::ModuleRegister<MODULE>::getInstance().getSelfRegistry().unregist(this);
  }

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @param arg2 first parameter
  /// @return SelfRegistPtr olding the created object
  Common::SelfRegistPtr<BASE> create(typename BASE::ARG1 arg1,
                                    typename BASE::ARG2 arg2)
  {
    return Common::SelfRegistPtr<BASE>(new CONCRETE(arg1, arg2), this);
  }

  /// Free an instance created by this factory
  /// @param ptr pointer to be freed
  void freeInstance ( void* ptr )
  {
    cf_assert(ptr != CFNULL);
    CONCRETE* obj = reinterpret_cast<CONCRETE*>(ptr);

    cf_assert(obj != CFNULL);
    delete_ptr<CONCRETE>( obj );
  }

}; // end of class ObjectProvider

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ObjectProvider_hh
