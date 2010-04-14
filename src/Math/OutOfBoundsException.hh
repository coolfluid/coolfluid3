#ifndef COOLFluiD_MathTools_OutOfBoundsException_hh
#define COOLFluiD_MathTools_OutOfBoundsException_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

  /**
   * This class represents an Exception thrown when a certain
   * value is not found in a storage or container.
   *
   * @author Andrea Lani
   *
   */
class MathTools_API OutOfBoundsException : public Common::Exception {
public:

  /**
   * Constructor
   * @parameter what  is the value that has been requested, but actually
   *                  doesn't exist
   *
   * @see COOLFluiD::Exception()
   */
  OutOfBoundsException (const Common::CodeLocation& where, const std::string& what) :
    Common::Exception(what)
  {
  }

  /**
   * Default destructor
   */
  ~OutOfBoundsException() throw()
  {
  }

  /**
   * The Exception name
   * Implement this function in each subclass.
   */
  virtual std::string getClassName() throw()
  {
    return "OutOfBoundsException";
  }

}; // end of class OutOfBoundsException

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_OutOfBoundsException_hh
