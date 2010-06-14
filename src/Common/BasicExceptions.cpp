#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

FailedAssertion::FailedAssertion (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "FailedAssertion")
{}

////////////////////////////////////////////////////////////////////////////////

BadValue::BadValue ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "BadValue")
{}

////////////////////////////////////////////////////////////////////////////////

CastingFailed::CastingFailed ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "CastingFailed")
{}

////////////////////////////////////////////////////////////////////////////////

FileFormatError::FileFormatError (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where,what,"FileFormatError")
{}

////////////////////////////////////////////////////////////////////////////////

FileSystemError::FileSystemError (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where,what,"FileSystemError")
{}

////////////////////////////////////////////////////////////////////////////////

FloatingPointError::FloatingPointError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "FloatingPointError")
{}

////////////////////////////////////////////////////////////////////////////////

ValueExists::ValueExists ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ValueExists")
{}

////////////////////////////////////////////////////////////////////////////////

ValueNotFound::ValueNotFound ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ValueNotFound")
{}

////////////////////////////////////////////////////////////////////////////////

ConvergenceNotReached::ConvergenceNotReached(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ConvergenceNotReached")
{}

////////////////////////////////////////////////////////////////////////////////

NotImplemented::NotImplemented(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotImplemented")
{}

////////////////////////////////////////////////////////////////////////////////

NotSupported::NotSupported(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotSupported")
{}

////////////////////////////////////////////////////////////////////////////////

ParallelError::ParallelError(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParallelError")
{}

////////////////////////////////////////////////////////////////////////////////

ParsingFailed::ParsingFailed ( const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParsingFailed")
{}

////////////////////////////////////////////////////////////////////////////////

SetupError::SetupError ( const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"SetupError")
{}

////////////////////////////////////////////////////////////////////////////////

ShouldNotBeHere::ShouldNotBeHere(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ShouldNotBeHere")
{}

////////////////////////////////////////////////////////////////////////////////

URLError::URLError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "URLError")
{}

////////////////////////////////////////////////////////////////////////////////

XmlError::XmlError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "XmlError")
{}

////////////////////////////////////////////////////////////////////////////////

InvalidStructure::InvalidStructure ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "InvalidStructure")
{}

////////////////////////////////////////////////////////////////////////////////

BuilderParserError::BuilderParserError(const CodeLocation& where, const string& what)
: Exception(where, what,"BuilderParser")
{}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

