#ifndef CF_Common_ConcreteProvider_hh
#define CF_Common_ConcreteProvider_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"


#include "Common/Provider.hpp"
#include "Common/Factory.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// @brief Concrete provider class for all types which the constructor takes zero arguments
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE, int NBARG = 0>
class ConcreteProvider : public Common::Provider<BASE> {
public:

  /// Constructor
  explicit ConcreteProvider(const std::string& name) :  Common::Provider<BASE>(name) {}

  /// Virtual destructor
  virtual ~ConcreteProvider() {}

  /// Polymorphic function to create objects of dynamical type BASE
  /// @return boost::shared_ptr olding the created object
  virtual boost::shared_ptr<BASE> create() = 0;

}; // end of class ConcreteProvider

////////////////////////////////////////////////////////////////////////////////

/// @brief Concrete provider class for all types which the constructor takes one argument
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class ConcreteProvider<BASE,1> : public Common::Provider<BASE> {
public:

  typedef BASE BASE_TYPE;
  typedef typename BASE::ARG1 BASE_ARG1;

  /// Constructor
  explicit ConcreteProvider(const std::string& name) :  Common::Provider<BASE>(name)  {}

  /// Virtual destructor
  virtual ~ConcreteProvider() {}

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @return boost::shared_ptr olding the created object
  virtual boost::shared_ptr<BASE> create(BASE_ARG1 arg1) = 0;

}; // end of class ConcreteProvider

////////////////////////////////////////////////////////////////////////////////

/// @brief Concrete provider class for all types which the constructor takes two argument
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class ConcreteProvider<BASE,2> : public Common::Provider<BASE> {
public:

  typedef BASE BASE_TYPE;
  typedef typename BASE::ARG1 BASE_ARG1;
  typedef typename BASE::ARG2 BASE_ARG2;

  /// Constructor
  explicit ConcreteProvider(const std::string& name) : Common::Provider<BASE>(name) {}

  /// Virtual destructor
  virtual ~ConcreteProvider() {}

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @param arg2 first parameter
  /// @return boost::shared_ptr olding the created object
  virtual boost::shared_ptr<BASE> create(BASE_ARG1 arg1, BASE_ARG2 arg2) = 0;

}; // end of class ConcreteProvider

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConcreteProvider_hh
