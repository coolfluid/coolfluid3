#ifndef CF_Common_NonCopyable
#define CF_Common_NonCopyable

//////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// Deriving from this class provides a clean and clear way to
/// show that a certain class is not copyable.
template < typename TYPE >
class NonCopyable {
public:

  /// Default inline constructor
  NonCopyable () {}

  /// Default inline destructor
  virtual ~NonCopyable () {}

private:

  /// private (non defined) copy constructor to prevent
  /// copying of the object
  NonCopyable (const NonCopyable & Source);

  /// private (non defined) assignment operator to prevent
  /// copy assignment of the object
  const NonCopyable & operator = (const NonCopyable & Source);

}; // end class NonCopyable

//////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_NonCopyable
