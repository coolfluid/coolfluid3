#ifndef COOLFluiD_Common_NonInstantiable_hh
#define COOLFluiD_Common_NonInstantiable_hh

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {
    namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// Derive from this class if you want a class that is not instantiable.
template < class TYPE >
class NonInstantiable {
private:
    NonInstantiable ();
};

//////////////////////////////////////////////////////////////////////////////

    } // namespace Common
} // namespace COOLFluiD

#endif // COOLFluiD_Common_NonInstantiable_hh
