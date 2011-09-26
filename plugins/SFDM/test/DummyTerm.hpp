// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_DummyTerm_hpp
#define CF_SFDM_DummyTerm_hpp

#include "SFDM/Term.hpp"

namespace CF {
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API DummyTerm : public Term {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< DummyTerm > Ptr;
  typedef boost::shared_ptr< DummyTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  DummyTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~DummyTerm();

  /// Get the class name
  static std::string type_name () { return "DummyTerm"; }

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

#endif // CF_SFDM_DummyTerm_hpp
