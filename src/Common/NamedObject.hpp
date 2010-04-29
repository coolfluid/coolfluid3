#ifndef CF_Common_NamedObject_hpp
#define CF_Common_NamedObject_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class represents an Object that can be named.
/// @author Tiago Quintino
class Common_API NamedObject {

public:

  /// Constructor with arguments
  explicit NamedObject(const std::string& name = std::string());
  /// Default destructor
  virtual ~NamedObject();

  /// Gets the name of the object.
  /// @return std::string with the object name.
  std::string getName() const { return m_name; }

protected: // functions

  /// Sets the object name
  /// @param name std::string with object name
  void setName(const std::string& name) {  m_name = name;  }

private: // data

  /// The object name stored as a std::string
  std::string m_name;

}; // end of class NamedObject

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_NamedObject_hpp
