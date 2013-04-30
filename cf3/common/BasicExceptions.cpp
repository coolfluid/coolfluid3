// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common {

////////////////////////////////////////////////////////////////////////////////

FailedAssertion::FailedAssertion (const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "FailedAssertion")
{}

FailedAssertion::~FailedAssertion () throw()
{}

////////////////////////////////////////////////////////////////////////////////

BadValue::BadValue ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "BadValue")
{}

BadValue::~BadValue () throw()
{}

////////////////////////////////////////////////////////////////////////////////

CastingFailed::CastingFailed ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "CastingFailed")
{}

CastingFailed::~CastingFailed() throw()
{}

////////////////////////////////////////////////////////////////////////////////

FileFormatError::FileFormatError (const common::CodeLocation& where, const std::string& what)
: common::Exception(where,what,"FileFormatError")
{}

FileFormatError::~FileFormatError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

FileSystemError::FileSystemError (const common::CodeLocation& where, const std::string& what)
: common::Exception(where,what,"FileSystemError")
{}

FileSystemError::~FileSystemError() throw()
{}


////////////////////////////////////////////////////////////////////////////////

FloatingPointError::FloatingPointError ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "FloatingPointError")
{}

FloatingPointError::~FloatingPointError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

SegmentationFault::SegmentationFault ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "SegmentationFault")
{}

SegmentationFault::~SegmentationFault() throw()
{}

////////////////////////////////////////////////////////////////////////////////
ValueExists::ValueExists ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "ValueExists")
{}

ValueExists::~ValueExists() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ValueNotFound::ValueNotFound ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "ValueNotFound")
{}

ValueNotFound::~ValueNotFound() throw()
{}

////////////////////////////////////////////////////////////////////////////////

FailedToConverge::FailedToConverge(const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"FailedToConverge")
{}

FailedToConverge::~FailedToConverge() throw()
{}

////////////////////////////////////////////////////////////////////////////////

NotImplemented::NotImplemented(const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotImplemented")
{}

NotImplemented::~NotImplemented() throw()
{}

////////////////////////////////////////////////////////////////////////////////

NotSupported::NotSupported(const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"NotSupported")
{}

NotSupported::~NotSupported() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ParallelError::ParallelError(const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParallelError")
{}

ParallelError::~ParallelError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ParsingFailed::ParsingFailed ( const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ParsingFailed")
{}

ParsingFailed::~ParsingFailed() throw()
{}

////////////////////////////////////////////////////////////////////////////////

SetupError::SetupError ( const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"SetupError")
{}

SetupError::~SetupError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ShouldNotBeHere::ShouldNotBeHere(const common::CodeLocation& where, const std::string& what)
: Exception(where, what,"ShouldNotBeHere")
{}

ShouldNotBeHere::~ShouldNotBeHere() throw()
{}

////////////////////////////////////////////////////////////////////////////////

URLError::URLError ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "URLError")
{}

URLError::~URLError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

XmlError::XmlError ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "XmlError")
{}

XmlError::~XmlError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

InvalidStructure::InvalidStructure ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "InvalidStructure")
{}

InvalidStructure::~InvalidStructure() throw()
{}

////////////////////////////////////////////////////////////////////////////////

IllegalCall::IllegalCall ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "IllegalCall")
{}

IllegalCall::~IllegalCall() throw()
{}

////////////////////////////////////////////////////////////////////////////////

NotEnoughMemory::NotEnoughMemory ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "NotEnoughMemory")
{}

NotEnoughMemory::~NotEnoughMemory() throw()
{}

////////////////////////////////////////////////////////////////////////////////

ProtocolError::ProtocolError ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "ProtocolError")
{}

ProtocolError::~ProtocolError() throw()
{}

////////////////////////////////////////////////////////////////////////////////

BadPointer::BadPointer ( const common::CodeLocation& where, const std::string& what)
: common::Exception(where, what, "BadPointer")
{}

BadPointer::~BadPointer() throw()
{}

////////////////////////////////////////////////////////////////////////////////

  } // common
} // cf3

