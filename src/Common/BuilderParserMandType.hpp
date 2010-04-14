#ifndef CF_Common_BuilderParserMandType_h
#define CF_Common_BuilderParserMandType_h

////////////////////////////////////////////////////////////////////////////////

#include "Common/CommonAPI.hpp"

namespace CF
{
namespace Common
{

////////////////////////////////////////////////////////////////////////////////

  /// @brief Defines available mandatoriness policies.

  /// @author Quentin Gasper

  enum BuilderParserMandType
  {
  /// @brief Undefined policy
  MAND_UNDEFINED,

  /// @brief The presence of an element is forbidden
  MAND_FORBIDDEN,

  /// @brief The presence of an element is optional.

  /// The element may be not present or present with an empty value.
  MAND_OPTIONAL,

  /// @brief The presence of an element is mandatory.

  /// The element must be present with a non-empty value.
  MAND_MANDATORY
  }; // enum BuilderParserMandType

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_BuilderParserMandType_h
