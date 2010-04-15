#ifndef CF_network_NetworkProtocol_h
#define CF_network_NetworkProtocol_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/BuilderParserRules.hpp"

#include "GUI/Network/NetworkAPI.hpp"
#include "GUI/Network/NetworkFrameType.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Defines the network protocol rules

  /// @author Quentin Gasper

  class Network_API NetworkProtocol : public CF::Common::BuilderParserRules
  {
    public:

    /// @brief Constructor
    NetworkProtocol();

    /// @brief Convert an unsigned int to the corresponding type.

    /// @param type The integer to convert
    /// @return Returns the converted type, or @c #NETWORK_NO_TYPE if the type
    /// could not be converted.
    NetworkFrameType convertToType(unsigned int m_type) const;

    /// @brief Convert a string to the corresponding type.

    /// @param type The string to convert
    /// @return Returns the converted type, or @c #NETWORK_NO_TYPE if the type
    /// could not be converted.
    NetworkFrameType convertToType(const QString & typeName) const;

    private:

    /// @brief Builds the rules
    void buildRules();

  }; // class NetworkProtocol

////////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_network_NetworkProtocol_h
