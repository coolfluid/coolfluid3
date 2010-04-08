#include "Common/BasicExceptions.hh"

//////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace CF {
  namespace Common {

//////////////////////////////////////////////////////////////////////////////

FailedAssertion::FailedAssertion (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "FailedAssertion")
{}

//////////////////////////////////////////////////////////////////////////////

BadValue::BadValue ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "BadValue")
{}

//////////////////////////////////////////////////////////////////////////////

CastingFailed::CastingFailed ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "CastingFailed")
{}

//////////////////////////////////////////////////////////////////////////////

FileFormatError::FileFormatError (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where,what,"FileFormatError")
{}
    
//////////////////////////////////////////////////////////////////////////////
    
FileSystemError::FileSystemError (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where,what,"FileSystemError")
{}
    
//////////////////////////////////////////////////////////////////////////////

FloatingPointError::FloatingPointError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "FloatingPointError")
{}

//////////////////////////////////////////////////////////////////////////////

NoSuchStorage::NoSuchStorage ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "NoSuchStorage")
{}

//////////////////////////////////////////////////////////////////////////////

NoSuchValue::NoSuchValue ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "NoSuchValue")
{}

//////////////////////////////////////////////////////////////////////////////

NotImplemented::NotImplemented(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotImplemented")
{}

//////////////////////////////////////////////////////////////////////////////

NullPointerError::NullPointerError(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NullPointerError")
{}

//////////////////////////////////////////////////////////////////////////////

ParallelError::ParallelError(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParallelError")
{}

//////////////////////////////////////////////////////////////////////////////

ParsingFailed::ParsingFailed ( const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParsingFailed")
{}

//////////////////////////////////////////////////////////////////////////////

SetupError::SetupError ( const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"SetupError")
{}

//////////////////////////////////////////////////////////////////////////////

ShouldNotBeHere::ShouldNotBeHere(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ShouldNotBeHere")
{}

//////////////////////////////////////////////////////////////////////////////

StorageExists::StorageExists ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "StorageExists")
{}

//////////////////////////////////////////////////////////////////////////////

URLError::URLError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "URLError")
{}

  } // namespace Common
} // namespace CF

