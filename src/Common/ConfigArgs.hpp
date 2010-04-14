#ifndef CF_Common_ConfigArgs_hh
#define CF_Common_ConfigArgs_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Definition of the ConfigKey type
typedef std::string ConfigKey;
/// Definition of the ConfigValue type
typedef std::string ConfigValue;
/// Definition of the map std::string to std::string type.
typedef std::map<ConfigKey,ConfigValue> ConfigMap;

/// This class represents an Arguments map.
/// It is used for storing arguments in Option parsing.
/// @author Tiago Quintino
class Common_API ConfigArgs : public ConfigMap {
public:

  /// Default constructor without arguments.
  ConfigArgs();

  /// Default destructor.
  ~ConfigArgs();

  /// Get these arguments in the form of string
  /// @returns string witht he arguments
  std::string str() const;

  /// Assignement operator for a map of strings
  ConfigArgs& operator=(const std::map<std::string,std::string>& value);

  /// Consume the configurations associated with the keys
  void consume (const std::vector<ConfigKey>& keys);

  /// Remove the configuration arguments that start with the provided key
  /// @todo this could use boost regexp
  void remove_filter (const ConfigKey& key);

  /// Only keep the configuration arguments that start with the provided key
  /// @todo this could use boost regexp
  void pass_filter (const ConfigKey& key);

}; // end of class ConfigArgs

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConfigArgs_hh
