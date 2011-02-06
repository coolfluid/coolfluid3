// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

FailedAssertion::FailedAssertion (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "FailedAssertion")
{}

FailedAssertion::~FailedAssertion () throw()
{}

////////////////////////////////////////////////////////////////////////////////

BadValue::BadValue ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "BadValue")
{}

BadValue::~BadValue () throw()
{}

////////////////////////////////////////////////////////////////////////////////

CastingFailed::CastingFailed ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "CastingFailed")
{}

CastingFailed::~CastingFailed() throw()
{}

////////////////////////////////////////////////////////////////////////////////

FileFormatError::FileFormatError (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where,what,"FileFormatError")
{}

FileFormatError::~FileFormatError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

FileSystemError::FileSystemError (const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where,what,"FileSystemError")
{}

FileSystemError::~FileSystemError() throw()
{}


////////////////////////////////////////////////////////////////////////////////

FloatingPointError::FloatingPointError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "FloatingPointError")
{}

FloatingPointError::~FloatingPointError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ValueExists::ValueExists ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ValueExists")
{}

ValueExists::~ValueExists() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ValueNotFound::ValueNotFound ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ValueNotFound")
{}

ValueNotFound::~ValueNotFound() throw()
{}

////////////////////////////////////////////////////////////////////////////////

FailedToConverge::FailedToConverge(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"FailedToConverge")
{}

FailedToConverge::~FailedToConverge() throw()
{}

////////////////////////////////////////////////////////////////////////////////

NotImplemented::NotImplemented(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotImplemented")
{}

NotImplemented::~NotImplemented() throw()
{}

////////////////////////////////////////////////////////////////////////////////

NotSupported::NotSupported(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotSupported")
{}

NotSupported::~NotSupported() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ParallelError::ParallelError(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParallelError")
{}

ParallelError::~ParallelError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ParsingFailed::ParsingFailed ( const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParsingFailed")
{}

ParsingFailed::~ParsingFailed() throw()
{}

////////////////////////////////////////////////////////////////////////////////

SetupError::SetupError ( const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"SetupError")
{}

SetupError::~SetupError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ShouldNotBeHere::ShouldNotBeHere(const Common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ShouldNotBeHere")
{}

ShouldNotBeHere::~ShouldNotBeHere() throw()
{}

////////////////////////////////////////////////////////////////////////////////

URLError::URLError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "URLError")
{}

URLError::~URLError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

XmlError::XmlError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "XmlError")
{}

XmlError::~XmlError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

InvalidStructure::InvalidStructure ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "InvalidStructure")
{}

InvalidStructure::~InvalidStructure() throw()
{}

////////////////////////////////////////////////////////////////////////////////

IllegalCall::IllegalCall ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "IllegalCall")
{}

IllegalCall::~IllegalCall() throw()
{}

////////////////////////////////////////////////////////////////////////////////

NotEnoughMemory::NotEnoughMemory ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "NotEnoughMemory")
{}

NotEnoughMemory::~NotEnoughMemory() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ProtocolError::ProtocolError ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "ProtocolError")
{}

ProtocolError::~ProtocolError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

BadPointer::BadPointer ( const Common::CodeLocation& where, const std::string& what)
: Common::Exception(where, what, "BadPointer")
{}

BadPointer::~BadPointer() throw()
{}

////////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF

